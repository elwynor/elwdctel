# Dice for Teleconference for The Major BBS v10

Dice rolling for the stock Major BBS v10 Teleconference. Roll dice in any
channel and everyone there sees the result. That includes users on other
boards in a linked channel such as MajorLink.

Dice for Teleconference is a conversion of **HOTELeconference Dice 2.00**
(1997) by Randall R. Spangler of annex.com. The original was an add-on for
annex's HOTELeconference; this version works with the Teleconference that
ships with The Major BBS v10.

## Features

- Adds `DICE` and `ROLL` commands to the stock Teleconference. There is no
  menu entry or anything else to set up.
- `ROLL 3D8` shows the roller their result in color. Everyone else in the
  channel sees one line of chat with the roll.
- **Works over linked channels.** The roll is sent as ordinary chat, so it
  reaches MajorLink (formerly WorldLink) and other linked boards, and they
  don't need Dice installed.
- Settings are changed online, with `DICE SET`, and take effect
  immediately. Changes are written to the Audit Trail.
- Limit dice to chosen channels, require a key, and set the most dice and
  most sides allowed.
- Rolls use the operating system's random number generator.

## Installation

1. Shut down the BBS.
2. Copy `ELWDCTEL.DLL`, `ELWDCTEL.MDF`, `ELWDCTEL.MSG` and `ELWDCTEL.RLN`
   from the release into the BBS folder.
3. Add the module in the configuration editor and start the BBS. The module
   adds `DICE` and `ROLL` to Teleconference.

Teleconference (GALTELE) is required. The module is a 32-bit (Win32) DLL,
like The Major BBS v10 itself, and runs on 64-bit Windows.

## Configuration

There are no configuration editor options. In Teleconference, a user with
the SYSOP key types `DICE SET` to list the settings, and
`DICE SET <number> <value>` to change one:

| # | Setting | Default |
|---|---|---|
| 1 | Dice commands work | Yes |
| 2 | ROLL is already on at sign-on | No |
| 3 | Most dice in one roll (1-100) | 50 |
| 4 | Most sides on one die (2-1000) | 100 |
| 5 | Key to use dice | blank (everyone) |
| 6 | Channels it works in (comma-separated) | blank (all) |

The text is in `ELWDCTEL.MSG` (level 6 in the configuration editor). See
`ELWDCTEL.RLN` for details.

## Usage

| Command | What it does |
|---|---|
| `DICE ON` | Allow `ROLL` |
| `DICE OFF` or `ROLL OFF` | Turn rolling off |
| `ROLL D20` | One 20-sided die |
| `ROLL 3D8` | Three 8-sided dice |
| `ROLL 4` | Four 6-sided dice |
| `DICE HELP` | Help |
| `DICE SET` | Sysop settings |

## Version Information

Current version: **1.0.0**. See [CHANGELOG.MD](CHANGELOG.MD).

## Tips / Donations

- **Cash App:** [$laudenbachm](https://cash.app/$laudenbachm)
- **Ko-fi:** [ko-fi.com/laudenbachm](https://ko-fi.com/laudenbachm)

## License

Licensed under the GNU Affero General Public License v3.0. See
[LICENSE](LICENSE).

Copyright © Elwynor Technologies. Originally HOTELeconference Dice by
Randall R. Spangler, annex.com.

## Support

Report problems through the GitHub issues of this repository.

Dice converted for The Major BBS v10 by Mark Laudenbach - https://github.com/laudenbachm
