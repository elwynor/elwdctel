# Changelog

All notable changes to Dice for Teleconference are recorded here. Versions
follow [Semantic Versioning](https://semver.org/).

## [1.0.0] - 2026-10-27

First release for The Major BBS v10, converted from HOTELeconference Dice
2.00 (ANXHDICE, 1997).

### Added

- The `DICE` and `ROLL` commands in the stock Teleconference.
- The roll reaches linked channels (MajorLink / WorldLink), because it is
  sent as an ordinary chat line.
- Online settings with `DICE SET`, stored in `ELWDCTEL.DAT`, with changes
  written to the Audit Trail.
- Settings to limit dice to chosen channels, to turn them off, to set the
  most dice and sides, and to have ROLL on at sign-on.
- Text uses named text variables (`DCT_*`).

### Changed (from the original)

- Removed the dependency on annex's HOTELeconference and its loader.
- The CNF options (KEYUSE, CMDDICE, CMDROLL, HLPTOPIC, ROOMTYPE) are
  replaced by online settings. The command words are now always `DICE` and
  `ROLL`. Room types became a channel list. The help topic is gone: help is
  `DICE HELP`.
- Rolls use the operating system's random number generator instead of a
  shuffled `rand()` table.
- The roll line names no pronoun; Teleconference already shows who sent it.

### Fixed (from the original)

- A user without the required key could get an undefined result from the
  `DICE` command.
- Extra characters after a roll (`3D8x`) were silently ignored; they are
  now refused.
