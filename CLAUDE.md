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
- Run `python check_readme.py` before pushing README or solution changes.

## Speed-evidence rules (learned live, 2026-08-15)

- The game's displayed speed is **asynchronous wall-time**, not a function
  of the simulator's frame timeline. Frame-identical A/B evidence does NOT
  establish displayed speed; only a live incumbent-vs-candidate A/B does.
- **Diagonal step substitutions are dead as a speed tool**: collapsing two
  cardinal steps into one diagonal regressed 36 to 41 live on two levels.
- Machine reach differs from the simulator: worker-to-worker gives reach
  diagonally, but machine gives serve cardinally, at the machine's front.
  A diagonal shredder give killed its givers live (Year 21), so never
  queue a merge that turns a machine give diagonal.
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
- An empty-handed give at a shredder feeds the worker to it, in any room
  where walking is allowed.  Never queue a program that can reach such a
  give, unless losing that worker cannot fail the level and the model,
  which includes the loss, still wins every run; the entry must say so.
- A give whose direction list resolves onto a printer destroys it and
  ends the run.  Give-list fall-through is real, so never queue a give
  list that can land on a printer.
- A pickup list does not skip an empty listed square (Year 60), so never
  rely on one doing so.
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
- A README row carries 📋 exactly when its program breaks one of these
  limits; `check_readme.py` enforces it, and queue entries follow the
  same rule in their headings.  COMMANDS.md has the details.
