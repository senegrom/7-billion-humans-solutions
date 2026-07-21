# 7BH emulator

A small C simulator that runs a 7 Billion Humans solution against a level and
reports whether it wins and how big it is. Purpose: machine-check candidate
solutions instead of clicking through the game.

This is a companion tool for people who **own** 7 Billion Humans. It reads none
of the game's files, but it refuses to run unless the game is installed — it is
only useful to owners, by design.

## Requires the game

At startup the emulator checks that 7 Billion Humans is installed (a Steam
install on any drive, or an existing save profile). If you own it but it isn't
detected, point at it explicitly:

```bash
export SEVENBH_GAME="D:/SteamLibrary/steamapps/common/7 Billion Humans/7 Billion Humans.exe"
```

## Build & test

```bash
bash build.sh              # -> ./emu.exe   (needs gcc; MinGW-w64 on Windows)
bash tests/run_tests.sh    # regression suite (must be all-green)
```

## Run

```bash
./emu.exe levels/year03_transport.lvl tests/year03.txt
```

Output reports `size` (command count — the game's SIZE metric) and `result`
(WIN/FAIL). It also prints raw lockstep `rounds`, which is **not** the game's
Speed score (see the caveat below).

## Level format (`levels/*.lvl`)

```
name <title>
dim  <width> <height>
row  <chars>          # one per grid row, top to bottom
...
goal <condition>
```

Grid characters: `.` floor · `#` wall · `O` hole/pit · `S` shredder ·
`P` printer · `G` goal pad (floor that must end holding a cube) · `@` worker
spawn · a digit/letter = a data cube of that value sitting on floor.

Goal conditions: `cubes_on_goals` · `shredded <n>` · `all_exited`.

## What is faithful, and what isn't

Faithful (verified against known-good repo solutions):

- Parser: labels, `comment`, `-- headers --`, comma-separated direction lists,
  `if <dir> ==/!= <type|number>` / `else` / `endif`, `jump`.
- Mechanics: `step`, `pickUp` (incl. `c` = current tile), `drop`, `giveTo`
  (shredder + worker hand-off), directional sensing (with the game's rule that
  a cube held aloft does not make a tile read as a datacube), holes as exits,
  wall/occupancy blocking.

Not yet faithful / deliberately refused at runtime rather than guessed:

- `takeFrom`, `calc`, `write`, `set`, `nearest`, `tell`, `listen`, `forEachDir`
  — parsed but abort execution (no silent mis-simulation).
- **The Speed metric.** The game's Speed is not "lines executed" (Year 2 = 1,
  Year 46 = 0); it needs calibration against the real game before we can rank
  speed solutions. We print raw rounds only as a coarse signal.
- **Real level grids.** The `.lvl` files here are hand-authored, self-consistent
  reconstructions (a level's known-good solution wins; obvious mutations lose),
  not the game's own level data.

So today the emulator is a correct engine for the mechanics it implements and a
solid regression harness; it is not yet a full-fidelity oracle for every level.
