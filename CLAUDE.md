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
  (`check_readme.solution_size`), not the emulator's counter.
- Run `python check_readme.py` before pushing README or solution changes.
