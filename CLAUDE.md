# Working rules for this repository

## Candidate win-rate floor

**Ignore solutions that win less often than 1 in 100 runs.** Do not queue
them in `SOLUTIONS_TO_TRY.md`, do not spend search time polishing them, and
do not ask for live-game attempts on them. A witnessed completion below
that rate costs more restart-grinding than the record row is worth.
Anything already found below the floor goes to the queue's
"Parked long shots" section (kept, not deleted, in case a higher-rate
variant turns up). Measured rates near the line (about 1%) stay queued.

## Live-game queue conventions

- `SOLUTIONS_TO_TRY.md` is the shared verification queue; every entry links
  a verified paste-ready program in `SolutionsToTry/`.
- An emulator win is evidence, not proof: a candidate leaves the queue only
  on a game completion screen (or an explicit live failure).
- The game stops every run at 1,400 seconds; completions past that are
  failures.
- Rejected and superseded experiments go to `REJECTED_APPROACHES.md` with
  the reason, so no effort repeats.
- Sizes quoted anywhere must be canonical editor sizes
  (`check_readme.solution_size`; the emulator counts the same way).
- Run `python check_readme.py` before pushing README, queue or solution
  changes; it also checks the queue's links, labels, sizes in file names
  and heading markers.

## Speed-evidence rules (learned live, 2026-08-15)

- The game's displayed speed is **asynchronous wall-time**, not a function
  of the simulator's frame timeline. Frame-identical A/B evidence does NOT
  establish displayed speed; only a live incumbent-vs-candidate A/B does.
- **Diagonal step substitutions are dead as a speed tool**: collapsing two
  cardinal steps into one diagonal regressed 36 to 41 live on two levels.
- Machine reach is not worker reach: a take from a printer is served at
  the printer's front (a diagonal take walks the worker there first),
  while a shredder is fed from any touching square, diagonals included
  (the Year 16-size, 19 and 21-size records feed diagonally).  Yet a merge
  that turned a shredder give diagonal killed its givers live (Year 21),
  for reasons not yet understood, so never queue a merge that turns a
  machine give diagonal.
- Simulator frame evidence remains valid for WIN/FAIL and for size.

## Candidate screening rules (learned live, 2026-08-19)

- **Items rule:** a deletion/edit candidate is only trustworthy when the
  emulator's item-action count matches the incumbent's.  Fewer frames
  plus fewer items = a different choreography that one scheduler order
  happened to survive (Year 20's live infinite loop).
- **Jitter rule:** screen every candidate under the shuffled-dispatch
  screen (the worker dispatch order shuffled every frame).  Interpret
  comparatively: the candidate must not do materially worse than the
  incumbent under the same screen; absolute 100% is only demanded where
  the incumbent holds it.  Levels whose published programs the model
  cannot reproduce cannot be screened; their entries are live-only and
  say so.

## Machine and list rules (learned live, 2026-08-17 to 2026-09-06)

- A step aimed at a shredder is refused: the worker stays put.
- An empty-handed give at a shredder feeds the worker to it, in every
  room, a no-walk room included, and so does an empty-handed takefrom
  aimed at a shredder (both decoded 2026-10-18).  Never queue a program
  that can reach such a give or take, unless losing that worker cannot
  fail the level and the model, which includes the loss, still wins every
  run; the entry must say so.
- A give list settles its target once, when the command comes up: a
  shredder, then a printer, then the first worker listed, else the first
  thing; a take list a printer, then a shredder, then the first worker.
  Nothing falls through to a later square (decoded 2026-10-18; the live
  Budget Brigade 2 loss read as a fall-through on 2026-09-06 is this
  rule).  A give that reaches a printer destroys it and ends the run, so
  never queue a give list that names a printer.
- A pickup list passes over bare squares, but the first thing it meets in
  slot order decides: a wall, hole, worker, or a counting machine's pad or
  button before the cube means no cube.  Year 04's 99+ `pickup c,e` starts
  on a bare square and completes; the fused Year 60 `pickup w,se` failed
  live because its leftmost worker's first listed thing is the wall to its
  west.
- A direction list is a set, stored in the slot order
  nw,w,sw,n,c,s,ne,e,se: a direction cannot be named twice, and the
  order in which a list is written carries no meaning.
- A label must be some jump's destination: deleting a jump deletes its
  label too, or the game refuses the paste.

## Editor limits and the paste marker (settled 2026-09-28)

- The editor gives `step` several directions only from Year 30, and
  never gives them to `pickup`, `giveto`, `takefrom` or `set`; such
  lists still work pasted in, on any level.  `myitem` is offered from
  Year 21; a pasted `myitem` on Year 15 did not work, so never queue a
  `myitem` program for Years 2-20.  Compound conditions (`and`/`or`)
  work wherever `if` does.
- A command missing from the level's palette (the table in COMMANDS.md)
  is a limit too: such a program can only be pasted in, and whether a
  paste keeps the command is the open Year 23 question.
- A README row carries 📋 exactly when its program breaks one of these
  limits; `check_readme.py` enforces it, and for the queue's headings
  too.  COMMANDS.md has the details.
