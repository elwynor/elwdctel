# Building Dice for Teleconference

Dice for Teleconference is a Win32 DLL module for The Major BBS v10. It is
built with Microsoft Visual Studio against the MBBS v10 Module SDK.

## Prerequisites

- **Microsoft Visual Studio 2022** (or Build Tools 2022) with the C++
  desktop workload. Platform toolset **v143**.
- **Windows 10 SDK** (`WindowsTargetPlatformVersion` 10.0).
- **The Major BBS v10 Module SDK**
  (https://github.com/TheMajorBBS/MBBS-v10-module-SDK).

> **Build for x86 (Win32) only. Never x64.** The Major BBS v10 is a 32-bit
> program and cannot load a 64-bit module.

## Setup

Copy `MBBSSDK.props.example` to `MBBSSDK.props` and set `MbbsSdkDir` to your
SDK folder (with the trailing backslash). `MBBSSDK.props` is not committed.

## Two things that differ from other modules

- **No Teleconference import library.** The module finds the four
  Teleconference functions it uses in `GALTELE.DLL` by name when the board
  starts (`DC_TLC.CPP`). The SDK's `GALTELE_LIB.LIB` names a different DLL
  and different export numbers than the real `GALTELE.DLL`, so a module
  linked with it does not load. Don't add that library to the project.
- **`TLCAPI.H` is not included.** One of the files it pulls in (`TLCACT.H`)
  does not compile with Visual Studio. `DC_TLC.H` includes only the
  Teleconference structure headers it needs.

## Message file

The module's text is in `SRC/ELWDCTEL/ELWDCTEL.MSG`, and
`SRC/ELWDCTEL/ELWDCTEL.H` holds the matching message numbers. Both are
generated from `ELWDCTEL.MST`, which spells control bytes as readable
tokens (`<ESC>`, `<TV:NAME>`, `<TV:NAME:C:12>`, `<HEX:DB*3>`). The `.MSG` must
keep CRLF line endings. Message numbers are assigned in file order,
starting from 0, and the `LEVEL6` marker takes slot 0. Deploy the DLL and
`.MSG` together; the BBS compiles `ELWDCTEL.MCV` itself.

## Build

Open `ELWDCTEL.sln`, choose **Release** / **Win32**, and build. Or:

```
MSBuild.exe ELWDCTEL.sln -p:Configuration=Release -p:Platform=Win32
```

The DLL is written to `BUILD/Release/ELWDCTEL.dll`.

## Tests

`TESTS\RUN_TESTS.CMD` builds and runs the unit tests for the text helpers
(`DC_UTIL.CPP`): reading a dice spec, the random numbers, the result list, the channel list, and
settings input. Exit code 0 means every test passed. The Teleconference
behaviour is tested on a BBS.

## Packaging a release

1. Set the new version in `SRC/ELWDCTEL/DC_DEFS.H` (`ELWDCTEL_VER_*`,
   `ELWDCTEL_VERSION`), `SRC/ELWDCTEL/ELWDCTEL.rc`, the `.MDF` and `.MST`
   headers, and `ELWDCTEL.RLN`. Update `CHANGELOG.MD`.
2. Rebuild the Release configuration, and check that the DLL's version
   matches `ELWDCTEL_VERSION`.
3. Copy only the deployable files into `DIST/<version>/`: `ELWDCTEL.DLL`,
   `ELWDCTEL.MDF`, `ELWDCTEL.MSG` and `ELWDCTEL.RLN`. Zip them as
   `ELWDCTEL-<version>.zip`.

Never rebuild under a version number that has already been released.
Bump the version instead.

## Release checklist

- The project builds with no errors or warnings, and the unit tests pass.
- Version numbers are updated (`DC_DEFS.H`, `ELWDCTEL.rc`, `CHANGELOG.MD`,
  `ELWDCTEL.RLN`).
- `DIST/<version>/` holds only the deployable files, and the MDF, MSG and
  RLN files have CRLF line endings.
- The DLL's properties show the right product, version, company and
  copyright.
- Installation is tested on a test board, including a linked channel.
- No credentials, internal paths or internal notes are in published files.
