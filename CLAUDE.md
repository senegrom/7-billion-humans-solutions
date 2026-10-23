# Working rules for this repository

## Candidate win-rate floor

**Ignore candidates that win fewer than 35% of their plain runs in the
model.**  The game keeps a completion only after its visible win: it then
plays 25 fresh random rounds, and if 12 or more fail it says the solution
failed more than 50 percent of the random tests and saves nothing.  That
check runs on every level from Year 09 on, and on Years 02-07 once Year
09 is beaten (decoded 2026-10-19), so a record needs a kept completion,
not just a visible win.  A program that wins a share p of its runs is
kept with a chance of about p × P(at least 14 of 25 rounds win) per
attempt: 1 in 1,000 at 28%, 1 in 100 at 35%, 1 in 6 at 50% and 4 in 5 at
80%.  Below 35%, one kept completion costs more than 100 attempts, more
restart-grinding than a record row is worth: do not queue such a
candidate in `SOLUTIONS_TO_TRY.md`, do not spend search time polishing
it, and do not ask for live-game attempts on it.  Anything already found
below the floor goes to the queue's "Parked long shots" section (kept,
not deleted, in case a higher-rate variant turns up).  Rates near the
line (30-40%) stay queued with their per-attempt figure, measured over
200 or more trials, since that figure moves steeply.

## Live-game queue conventions

- `SOLUTIONS_TO_TRY.md` is the shared verification queue; every entry links
  a verified paste-ready program in `SolutionsToTry/`.
- An emulator win is evidence, not proof: a candidate leaves the queue only
  when the game keeps a completion (its panel finishes the 25 random
  rounds and records the size and speed) or on an explicit live failure.
  A visible win followed by the "more than 50 percent" message is a live
  failure.  After any failed round, kept or not, the game replays that
  room on the next run until you leave the level, so go back to the
  elevator before counting another attempt.
- The game stops every run at 1,400 seconds, the visible run and each of
  the 25 rounds alike; completions past that are failures.
- Rejected and superseded experiments go to `REJECTED_APPROACHES.md` with
  the reason, so no effort repeats.
- Sizes quoted anywhere must be canonical editor sizes
  (`check_readme.solution_size`; the emulator counts the same way).
- Run `python check_readme.py` before pushing README, queue or solution
  changes; it also checks the queue's links, labels, sizes in file names
  and heading markers.

## Speed-evidence rules (learned live 2026-08-15, explained 2026-10-19)

- The displayed speed is not wall time: the game counts a fixed 16 ms per
  tick, shows the whole seconds of the mean over the winning rounds of its
  25 random tests, and keeps the best it has shown (decoded 2026-10-19).
  The model's estimate is the whole seconds of 0.016 s × frames, averaged
  over winning trials.  Its timing is not exact everywhere, though: in the
  2026-08-15 Year 39 and 40 A/Bs, programs it ran in identical frames
  showed 41 s against 36 s live.  So frame-identical A/B evidence does NOT
  establish displayed speed; only a live incumbent-vs-candidate A/B does.
- **Diagonal step substitutions are dead as a speed tool**: collapsing two
  cardinal steps into one diagonal regressed 36 to 41 live on two levels.
- Machines serve from their front square only: a take from a printer is
  served there (a diagonal take walks the worker there first), and so is
  a give to a shredder: a giver beside it or at its corner walks onto the
  front first and stays there (decoded 2026-10-19; the Year 16-size, 19
  and 21-size records reach their diagonal shredders so).  Whatever the
  giver does next it does from the front, so audit every command a
  shredder giver runs after its give.  That is what killed the Year 21
  merge's givers live: its `takefrom s` then named the shredder, and an
  empty-handed take there feeds the worker to it (the model now loses
  that merge 195 runs in 200).  The records' paste-only `takefrom sw,s`
  (Year 19) and `takefrom s,se` (Year 21 size) settle on the printer
  first and keep their workers alive; their `takefrom s` forms lose
  every run.
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
  say so.  The game runs its workers in one fixed order every tick, the
  emulator's plain order, so the plain rate is the model's own rate; the
  shuffled screen measures how much a choreography leans on exact timing
  (Year 44's speed record wins every plain run and about one shuffled run
  in twenty).

## Machine and list rules (learned live, 2026-08-17 to 2026-09-06)

- A step aimed at a shredder is refused: the worker stays put.
- An empty-handed give at a shredder feeds the worker to it, in every
  room, a no-walk room included, and so does an empty-handed takefrom
  aimed at a shredder (both decoded 2026-10-18); the worker walks onto
  the shredder's front first, from beside it or, when the shredder is
  remembered, from across the room (decoded 2026-10-19).  Never queue a
  program that can reach such a give or take, unless losing that worker
  cannot fail the level and the model, which includes the loss, still
  wins every run; the entry must say so.
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
- A take, give, pickup or set aimed by direction lists at most five
  things, square by square in that slot order (decoded 2026-10-19).  A
  worker, machine, the boss or a pad met once five are listed goes
  unseen; a wall, hole or floor cube met then overruns the list.  A take
  survives an overrun only with a printer among the five, a give only
  with a shredder among them and a cube in hand, a pickup only if its
  first thing is not a machine, and a set never; a second overrun always
  crashes the game, and the run is lost.  Never queue a direction list
  that can crash so: the Year 30 eight-sided take does in every run.
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
