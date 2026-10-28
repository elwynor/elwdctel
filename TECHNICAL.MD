# Dice for Teleconference - technical notes

## Overview

ELWDCTEL is a background module: its MDF is `UNCONDITIONAL` and `INTERNAL`,
and it has no menu entry. Its init finds the Teleconference functions and
opens its files. It then registers a startup hook (`hook_startup`), which the
board runs after every module's init and before the main loop. That hook adds
the `DICE` and `ROLL` commands to the stock Teleconference (GALTELE), so
module load order doesn't matter. From then on, Teleconference calls the
module whenever a user types either word.

| File | What it holds |
|---|---|
| `ELWDCTEL.CPP` | Start-up and shut-down, the two commands, the roll, the settings editor, text variables |
| `DC_TLC.CPP/H` | The link to the Teleconference engine (GALTELE.DLL, found by name) |
| `DC_SET.CPP/H` | The settings record and its descriptions |
| `DC_UTIL.CPP/H` | Helpers with no BBS dependencies (unit tested) |
| `ELWDCTEL.MST` | Message template; `TOOLS/MKMSG.PY` builds `.MSG` and `.H` from it |

## The Teleconference API used

Four GALTELE.DLL exports, found at start-up with `GetProcAddress`:
`_init__galtele`, `_tlcCommandRegister`, `_tlcUserSendText` and
`_tlcChannelPublicSendBut`. The SDK's import library and `TLCAPI.H` are not
used (see BUILDING.MD). The Teleconference keeps a pointer to each
`CMDSTRUCT`, so both are static. The engine has no call to remove a command,
which is why `DICE` and `ROLL` are fixed words. A command handler that returns
1 tells Teleconference the line was handled.

## A roll

`ROLL` text goes to `dc_parsedice`, which accepts `[n][D sides]` with spaces
and case ignored, defaults to one die of six sides, and refuses anything left
over. Counts are checked against the `maxdice` setting and sides against
`maxsides`. Each die is `1 + dc_rand(sides)`, where `dc_rand` uses `rand_s`
and rejects the biased top range.

The roller is sent a private colored version (`ME1`, `MEN` or `MENT`) with
`RECV_TYPE_PRFBUF`. Everyone else gets one plain line (`PUB1`, `PUBN` or
`PUBNT`) from `tlcChannelPublicSendBut` with `RECV_TYPE_PUBLIC` and message
number 0, so it crosses linked channels and reads after "From <user>:" on
other boards. The roller is excluded from the public send. If the list of
results doesn't fit `DC_LISTSIZ` characters, the `...T` variants show only
the total.

## Text variables

All dynamic text uses named text variables with the `DCT_` prefix: `NDICE`,
`SIDES`, `TOTAL`, `ROLLS`, `CHANNEL`, `MAXN`, `MAXS`, `DICE`, `ROLL`,
`VERSION`, `SETNUM`, `SETNAME`, `SETVALUE`, `SETRANGE`.

## Settings record

`ELWDCTEL.DAT` holds one 512-byte record with key `"SETTINGS"` (key 0, a
zero-terminated string). The page size is 1024. The file is created with
`dfaCreateSpec` on first start. The record struct is packed to one byte. New
settings take their room from `spare[]`. Settings are read at the point of
use, never cached, and the lookup key is always copied to writable memory.

| Field | Type | Default |
|---|---|---|
| `layout` | SHORT | 1 |
| `enabled` | SHORT | 1 |
| `autoon` | SHORT | 0 |
| `maxdice` | SHORT | 50 (1-100) |
| `maxsides` | SHORT | 100 (2-1000) |
| `usekey` | CHAR[48] | "" (everyone) |
| `channels` | CHAR[200] | "" (all) |

## Per-user state

`rollon[nterms]` holds one byte per channel: ROLL switched on. It is set from
the `autoon` setting at logon and cleared at hang-up, because the board
reuses a channel slot as soon as a caller drops.
