# Solutions to Try in the Real Game

This is the live-game verification queue.  A candidate stays here until the
game itself reaches its completion screen (or an explicit failure), because an
emulator win is evidence rather than proof.  The game stops a run after 1,400
seconds, so an unfinished run at that point is a failure, not an eventual win.

For every attempt, record the editor-reported size, completion/failure, the
displayed speed, and a screenshot.  For stochastic programs, also record the
attempt number and do not restart merely because a run looks slow.

Rejected and superseded experiments are archived in
[REJECTED_APPROACHES.md](REJECTED_APPROACHES.md).

**Speed-evidence downgrade (2026-08-15):** two live A/Bs (Years 39 and
40) proved the game's displayed speed is asynchronous wall-time —
frame-identical simulator evidence does NOT establish it, and diagonal
step substitutions regressed 36 to 41 in both tests.  Every speed
tie-break below therefore requires a live incumbent control run first;
discard the candidate on any displayed-speed regression.  Win/loss and
size evidence is unaffected.

Rates are quoted at the live 1,400 s clock, measured plain and under the
shuffled-dispatch screen (the worker dispatch order shuffled every
frame).

Every entry links a **paste-ready program file** in
[SolutionsToTry/](SolutionsToTry/) — open it, select all, copy, and paste
into the level's editor.  Recipes and evidence below describe how each
file was derived and verified.

## Priority queue

**Queue refresh (2026-09-28):** the search fleet stays stopped, and the
list below runs from the cheapest attempt to the most expensive, with the
optional items last.

**Withdrawn for safety (2026-10-18):** the game cannot handle a take,
give, pickup or set whose direction list gathers six or more things
(walls, holes and cubes all count): it faults.  The Year 30 eight-sided
take does that beside the printer in every run, so it is withdrawn and
must not be pasted; the parked Year 06 eight-square pickup goes with it.

**Paste markers settled (2026-09-28):** the editor's limits are now
known from the game itself (see COMMANDS.md): several directions on
`step` only from Year 30, never on `pickup`, `giveto`, `takefrom` or
`set`, and `myitem` from Year 21, one level earlier than these notes
used to say.  So the housekeeping look at the editor is dropped, every
entry below that has to be pasted now carries 📋, and the Year 23 paste
test's reasoning is corrected: no operand or command missing from a
level's editor had in fact been shown to survive a paste.

**Goal checks corrected (2026-10-16):** several of the model's goal
checks were corrected, and three entries change with them.  Injection
Sites counts a row only when it is whole, Injection Sites and Good
Neighbors stay open while anybody still holds a cube, and Local Maximums
wants every shredded number to be the largest some group started with.
So the Year 13 six, the Year 56 tier check and the Year 58 four (now
0/1000, with its published Solutions50+ row) are withdrawn; see
REJECTED_APPROACHES.md.

**Model re-measured (2026-10-18, after a review of the emulator against
the game's own code):** every queued program was measured again on the
corrected model, and the entries below quote the new figures.  Two
working rules change with it (see CLAUDE.md and the 2026-10-18 section of
REJECTED_APPROACHES.md): no give or take list falls through, so a give
list that names a printer goes to the printer, and an empty-handed give
or take at a shredder kills in every room.  The Year 62 list merge at 213
is withdrawn: it now does materially worse than its incumbent.  The Year
44 transient-survivor three leaves the parked long shots.

**Next session (50%+ rule in force — no low-percent testing for now):**

1. **Year 23 paste test, size 2**: one paste and one look at the editor,
   about a minute.  It decides whether the game keeps a command the
   level's editor does not offer; if the run then completes, it is a
   size-2 record against 6.  Expect a no: the one hidden feature
   pasted before (`myitem` on Year 15) did not work.
2. **Only if item 1 kept its command:** the Year 16 four (record 6) and
   the Year 15 five (records 8 and 6), each a couple of short runs.
3. **Speed tie-breaks**, each with the incumbent control run first, every
   run under a minute: Year 38 at 140, Year 09 at 14, Year 59 at 142,
   Year 58 at 102, Year 40 at 93 (yifeixuer's 2018 program, live-only),
   then the Year 62 ladder and the Year 67 ladder.  The Year 68 ladder
   waits until its control completes: the published program failed all
   three attempts on 2026-08-22.
4. Optional control: the Year 15 community six, already the Solutions50+
   row on public evidence (16/25 live); about 81% here.  Skip it if the
   Year 15 five in item 2 completes.
5. Optional, for a patient session: confirmations of the published Year 38
   community speed 6-7 and Year 30 probabilistic four; and the Year 30
   alternate five, an unpublished edit of the size-5 row whose only claim
   is a small model speed edge that a live A/B has to confirm.

Low-percent leads are parked in their own section further down until you
ask for that tier again.

### [ ] Year 23 - Sorting Hall - paste test for a command the editor lacks at size 2 📋

- **Paste-ready program:** [SolutionsToTry/Year 23 - Sorting Hall - paste test for a command the editor lacks at size 2.txt](<SolutionsToTry/Year 23 - Sorting Hall - paste test for a command the editor lacks at size 2.txt>)
- Goal: settle a question that has never been tested, and take the size
  record from **6** to **2** if the answer is yes.  Pasted direction
  lists work on every level, but a list is only a form the editor's
  picker refuses.  Whether a paste keeps a whole *command* the level's
  editor does not offer is still open, and the one hidden operand
  tried so far did not work (`myitem` on Year 15, before its
  Year 21 unlock; see REJECTED_APPROACHES.md).  This program uses
  `write`, which the editor first offers at Year 32.  CONTRIBUTING.md
  already lists pasted programs that use "a command the game's editor
  will not let you build at that level", so a completion would count.
- Mechanism: every worker lifts the cube below it and writes 0 on it.
  All the held cubes then read 0, and a row of equal numbers is in order.
- Emulator evidence, with the level's command palette lifted because
  that is exactly the question: **1000/1000 plain and 1000/1000 under
  the shuffled-dispatch screen**, finishing in about 1.5 s of game time.
- Suggested live test:
  1. Paste, then look at the editor *before* running.  If the `write 0`
     line is missing, or the paste is refused, the game enforces the
     palette on paste: mark this entry refuted and skip the next two.
  2. If the line survived, run it.  A completion is a size-2 record.  If
     it runs but never completes, the paste kept the command but the goal
     wants more than equal numbers; try the next two entries anyway,
     since neither relies on that.
- Left out on purpose: the same trick also wins in the model on Budget
  Brigade 2 at size 4 (the all-left relay plus `write 0`), but its give
  list can land on a printer, which is how the relay six died in the
  game.
- Result: _not yet tested in the game_.

### [ ] Year 16 - Little Exterminator 2 - nearest one-shot at size 4 📋

- **Paste-ready program:** [SolutionsToTry/Year 16 - Little Exterminator 2 - nearest one-shot at size 4.txt](<SolutionsToTry/Year 16 - Little Exterminator 2 - nearest one-shot at size 4.txt>)
- **Try this only if the Year 23 paste kept its command.**  It uses
  `nearest`, which the editor first offers at Year 25.
- Goal: size **4** against the record of 6, which is also the game's par.
- Mechanism: Neural Pathways' published four, unchanged.  Each of the
  three workers takes the nearest cube and feeds it to the nearest
  shredder.  The room holds exactly three cubes, so one trip each
  finishes the level.
- Emulator evidence, level palette lifted: **1000/1000 plain and
  1000/1000 shuffled**, about 5 s of game time.  The three workers always
  pick three different cubes.
- Suggested live test: paste, confirm editor size 4 with both `nearest`
  lines present, and run it twice.
- Result: _not yet tested in the game_.

### [ ] Year 15 - Shred Lines - nearest loop at size 5 📋

- **Paste-ready program:** [SolutionsToTry/Year 15 - Shred Lines - nearest loop at size 5.txt](<SolutionsToTry/Year 15 - Shred Lines - nearest loop at size 5.txt>)
- **Try this only if the Year 23 paste kept its command.**
- Goal: size **5** against the 99%+ record of 8 and the Solutions50+ six.
- Mechanism: the published five of My First Shredding Memory and
  Biometric Access, unchanged.  Each worker remembers its nearest
  shredder once, then keeps taking the nearest cube and feeding that
  shredder.
- Emulator evidence, level palette lifted: **1000/1000 plain and
  1000/1000 shuffled**, about 16 s of game time (re-measured 2026-10-18;
  it was about 22 s while the model let `nearest datacube` find cubes in
  other workers' hands, which the game does not).
- Caution: a worker that loses a race for a cube still walks to its
  shredder and, arriving empty-handed, is lost; that is the mechanism
  behind the earlier Shred Lines failures.  In the model the other
  workers always finish the job, but stop at the first failed run.  It
  is queued under the exception in the working rules: losing a worker
  cannot fail this level, and the model, which includes the loss, wins
  every run.
- Suggested live test: paste, confirm editor size 5, and run it two or
  three times.
- Result: _not yet tested in the game_.

### [ ] Year 38 - Seek and Destroy 3 - speed tie-break at size 140 (fallback 141)

- **Paste-ready program:** [SolutionsToTry/Year 38 - Seek and Destroy 3 - speed tie-break at size 140.txt](<SolutionsToTry/Year 38 - Seek and Destroy 3 - speed tie-break at size 140.txt>)
- **Fallback (size 141):** [SolutionsToTry/Year 38 - Seek and Destroy 3 - speed tie-break fallback at size 141.txt](<SolutionsToTry/Year 38 - Seek and Destroy 3 - speed tie-break fallback at size 141.txt>)
- Goal: retain the displayed 9-10 while reducing the size from 142 to
  **140**.
- Exact edit: inside the second `if mem3 != mem4:` block, the column walk
  goes `step w`, five `step n`, `step e`, `step w`.  The first `step w`
  and the `step e` cancel (identical endpoint), so both are deleted; the
  fallback deletes only the `step w`.
- Emulator evidence: 199/200 wins at 606 frames versus the incumbent's
  620 (the fallback: 200/200 at 614).  Item-action count identical.
  Re-screened 2026-09-03 with the build that implements the screen:
  199/200 for the candidate and 200/200 for the fallback, frames and
  item counts unchanged.  Re-measured 2026-10-18 over 1000 worlds: the
  candidate 994 plain and 993 shuffled at 608 frames, the fallback 998
  on both at 617, the incumbent 998 on both at 618, item counts within
  0.1 of each other.
- Suggested live test: incumbent once as control, then the candidate;
  9-second runs.
- Result: _not yet tested in the game_.

### [ ] Year 09 - Dynamic Angles - speed tie-break at size 14

- **Paste-ready program:** [SolutionsToTry/Year 09 - Dynamic Angles - speed tie-break at size 14.txt](<SolutionsToTry/Year 09 - Dynamic Angles - speed tie-break at size 14.txt>)
- Goal: retain the displayed speed of 3 while reducing the size from 15 to
  **14** (martinez8859, n05ucc4u and abfipes12's program; keep the credits).
- Exact edit: delete the first `jump a` (inside the first `if e == nothing:`
  block) **and the label `a:` it pointed to** (labels are free, so the size
  is unchanged).  Without the jump, the workers on the longest route re-test
  `e == nothing` at each following block instead of jumping past the test;
  on this level's diagonal edge those tests are always true, so the walk is
  the same.
- **Paste-validity fix after the first live attempt (2026-08-17):** the
  first cut deleted only the jump and left `a:` behind, and the game
  refused the paste — a label can only exist as some jump's destination,
  so an orphaned label makes the whole program invalid.  The paste file
  now removes the pair.  (Standing rule for deletion candidates: a deleted
  jump takes its label with it.)
- Emulator evidence (corpus deletion sweep, re-run on the fixed file):
  100/100 wins at exactly the incumbent's 230.0 frames, and 200/200
  under the shuffled-dispatch screen re-taken 2026-09-03.  Re-measured
  2026-10-18: 1000/1000 plain and shuffled for both, at 230 frames.  The
  candidate counts 25 item actions to the incumbent's 24; the one extra
  is the `if e == nothing` re-test the deleted jump used to skip, with
  every step, pickup and drop the same.  The only live
  risk is that the three extra tests cost wall time on the longest route
  (Year 47 showed an `if` is not free live) — a 3-second A/B decides it.
- Suggested live test: incumbent once as control, then the candidate.
- Result: _first paste refused (orphaned label, 2026-08-17); the fixed
  file has not been run yet_.

### [ ] Year 59 - Glory Hole - speed tie-break at size 142

- **Paste-ready program:** [SolutionsToTry/Year 59 - Glory Hole - speed tie-break at size 142.txt](<SolutionsToTry/Year 59 - Glory Hole - speed tie-break at size 142.txt>)
- Goal: retain the displayed 6 while reducing the size from 144 to
  **142**.
- Exact edit: in the else-arm walk `step w / step sw / step sw / step sw /
  step e`, the `step w` and `step e` cancel — the three diagonals land on
  the same square, two commands shorter.
- Emulator evidence: 300/300 wins, deterministic at 455 frames versus the
  incumbent's 447 — 8 frames slower in the model, so the displayed speed
  needs the live A/B (control run first, discard on regression).
  Item-action count identical; 200/200 under the shuffled-dispatch
  screen re-taken 2026-09-03, at the same 455 frames.  Re-measured
  2026-10-18: 1000/1000 plain and shuffled at 452 frames against the
  incumbent's 443, still 9 frames slower.
- Result: _not yet tested in the game_.

### [ ] Year 58 - Good Neighbors - list-merge speed tie-break at size 102 📋

- **Paste-ready program:** [SolutionsToTry/Year 58 - Good Neighbors - list-merge speed tie-break at size 102.txt](<SolutionsToTry/Year 58 - Good Neighbors - list-merge speed tie-break at size 102.txt>)
- Goal: retain the displayed speed record of 2 while reducing the size
  from 105 to **102**.
- Exact edit: the `if mem2 == nothing: pickup ne / else: pickup nw /
  endif` block becomes the one list `pickup nw,ne`.  A pickup list passes
  over a bare square and takes the first thing it meets in slot order, so
  the list lifts the north-west cube when it is there and the north-east
  one otherwise, as the block did.
- Emulator evidence (2026-10-18, on the corrected model): 1000/1000
  plain and 1000/1000 shuffled, frame-identical to the incumbent at 230
  frames with the same item-action count (38).
- History: this merge was closed on 2026-08-17 on the old reading of the
  Year 60 failure (that a list never passes over an empty square); the
  corrected reading reopens it.  Paste-only: the editor gives `pickup` no
  lists.
- Suggested live test: incumbent once as control, then the candidate;
  2-second runs; discard it on any displayed-speed regression.
- Result: _not yet tested in the game_.

### [ ] Year 40 - Printing Etiquette 2 - yifeixuer's speed program at size 93

- **Paste-ready program:** [SolutionsToTry/Year 40 - Printing Etiquette 2 - yifeixuer speed at size 93.txt](<SolutionsToTry/Year 40 - Printing Etiquette 2 - yifeixuer speed at size 93.txt>)
- Goal: match the speed record of 36 at size **93**, against the
  incumbent's 176.
- Source: yifeixuer's Steam post "Year 40 36s" of 2018-10-06
  ([thread](https://steamcommunity.com/app/792100/discussions/0/1728711392737304334/)),
  from game version 2145.  The same day commonnickname withdrew the
  176-command edit (upstream pull request #92) in its favour: "Solution
  by yifeixuer looks much better and he got it first."  It never reached
  either repository.
- Emulator evidence: none usable.  The model loses it 0/200 plain and
  0/200 shuffled while winning the incumbent 1000/1000 (2026-10-18), but
  Printing Etiquette is one of the levels whose crowd behaviour the model
  is known to miss, so only the game can judge it.  Queued live-only at the maintainer's
  request.
- Suggested live test: run the incumbent once as the control, then this
  program a few times; capture the completion panel, the displayed speed
  and the editor size.  A completion at 36 s or faster takes the speed
  row at size 93.
- Result: _not yet tested in the game_.

### [ ] Year 68 - Goodbye, Humans! - tell-only speed tie-break at size 170 (fallback 171)

- **Paste-ready program:** [SolutionsToTry/Year 68 - Goodbye, Humans! - tell-only speed tie-break at size 170.txt](<SolutionsToTry/Year 68 - Goodbye, Humans! - tell-only speed tie-break at size 170.txt>)
- **Fallback (size 171):** [SolutionsToTry/Year 68 - Goodbye, Humans! - tell-only speed fallback at size 171.txt](<SolutionsToTry/Year 68 - Goodbye, Humans! - tell-only speed fallback at size 171.txt>)
- Goal: retain the displayed speed record of 16 while reducing the secondary
  size from 172 to **170** (or 171 at the conservative rung).
- Exact edit: delete both consecutive top-level `tell everyone hi` commands at
  incumbent lines 195-196; restore either one for the size-171 fallback.  No
  `listenfor` exists, so the deleted greetings carry no data and only change
  asynchronous cadence.
- Why this is reopened: the size-171 form was queued originally, then 171/170
  moved to rejected only because the nominally stronger bypassed-wrapper
  size-164 program dominated them.  Size 164 later failed live, leaving the
  tell-only rungs needing an independent test.
- Live ladder: run the incumbent size-172/speed-16 program as control, then 171,
  then 170.  Stop at the first failure or displayed speed above 16.
- Live-only: the model does not reproduce the incumbent (0/40 worlds), so
  no emulator screen applies to these rungs.
- Result: _live attempt inconclusive_.  The published size-172 control failed
  all three attempts on 2026-08-22, so it did not establish a passing
  baseline.  The correctly pasted size-171 rung then failed its one attempt;
  size 170 was not run under the stop-on-failure rule.  Keep this queued for a
  future session that first obtains a successful control run.

### [ ] Year 62 - The Sorting Floor - duplicate-store speed tie-break at size 214 (fallback 215)

- **Paste-ready program:** [SolutionsToTry/Year 62 - The Sorting Floor - duplicate-store speed tie-break at size 214.txt](<SolutionsToTry/Year 62 - The Sorting Floor - duplicate-store speed tie-break at size 214.txt>)
- **Fallback (size 215):** [SolutionsToTry/Year 62 - The Sorting Floor - duplicate-store fallback size 215.txt](<SolutionsToTry/Year 62 - The Sorting Floor - duplicate-store fallback size 215.txt>)
- Goal: retain the current displayed speed range of 9-12 while reducing the
  secondary size from 216 commands to 214, with the tested one-store deletion
  at size 215 as the fallback.
- Exact edit: start from
  [the current speed program](<Solutions99+/Year 62 - The Sorting Floor (speed).txt>)
  and delete both consecutive `mem1 = set myitem` commands immediately before
  `tell everyone hi` and the divide-by-zero exit.  To test the size-215
  fallback, restore either one of them.
- Why it may be safe: neither saved value is read before that worker's exit;
  only the two synchronization delays can matter.
- Fallback emulator A/B evidence: the size-215 candidate and incumbent both
  win seed 1 in exactly 836 frames.  Across the same 100 model worlds each
  wins 36, with modelled speed 12.3 and nearly identical average frames
  (734.9 versus 735.0).
- Size-214 bounded gate: 9/20 model worlds won, with modelled speed averaging
  10.9 (range 6-14), winning frames averaging 648.0 (range 371-836), and 40.4
  average item actions.  The observed 45% is encouraging but not statistically
  decisive against the established 36/100 baseline.
- Live-only: the model under-reproduces the published incumbent (15/40
  worlds), so those paired aggregates support equivalence but cannot
  establish live reliability or timing.
- Re-measured 2026-10-18 over 1000 worlds on the corrected model: the
  incumbent wins 150 plain and 145 shuffled, size 215 wins 150 and 128,
  and size 214 150 and 118.  Plain they are frame-identical; under the
  shuffled screen the rungs trail the incumbent, the 214 by about two
  standard errors, so stop the ladder at the first live regression.
- Expected editor size: **214** if the extra cadence cut survives; otherwise
  use the tested **215** fallback.
- Suggested live test: run the incumbent, size 215, and size 214 in that order;
  stop at the first regression and capture each completion panel and editor
  size.
- Result: _not yet tested locally in the game_.

### [ ] Year 67 - Decimal Doubler - live-only speed tie-break at size 205 (fallbacks 206-209)

- **Paste-ready program:** [SolutionsToTry/Year 67 - Decimal Doubler - live-only speed tie-break at size 205.txt](<SolutionsToTry/Year 67 - Decimal Doubler - live-only speed tie-break at size 205.txt>)
- **Fallback (size 206):** [SolutionsToTry/Year 67 - Decimal Doubler - fallback size 206.txt](<SolutionsToTry/Year 67 - Decimal Doubler - fallback size 206.txt>)
- **Fallback (size 207):** [SolutionsToTry/Year 67 - Decimal Doubler - fallback size 207.txt](<SolutionsToTry/Year 67 - Decimal Doubler - fallback size 207.txt>)
- **Fallback (size 208):** [SolutionsToTry/Year 67 - Decimal Doubler - fallback size 208.txt](<SolutionsToTry/Year 67 - Decimal Doubler - fallback size 208.txt>)
- **Fallback (size 209):** [SolutionsToTry/Year 67 - Decimal Doubler - conservative fallback size 209.txt](<SolutionsToTry/Year 67 - Decimal Doubler - conservative fallback size 209.txt>)
- Goal: retain the current displayed speed record of 41 while reducing the
  secondary size from 210 commands to 205.
- Exact edits: after opening `pickup ne; step n`, delete all three consecutive
  `mem2 = set c` commands; in loop `j`, after `step mem3; step e`, delete both
  consecutive `tell everyone hi` commands.
- Why they may be safe: every `mem2` occurrence is an assignment, never a read,
  and the program contains no `listenfor`; all five deleted commands are
  cadence only.  The ladder removes one of them per rung: 209 deletes one
  tell, 208 one store as well, 207 the second tell, 206 a second store,
  and 205 the third.
- Timing caveat: candidate and incumbent both fail in the current model, so
  it cannot referee either edit; this remains live-only.
- Expected editor size: **205**; expected displayed speed: **41**.
- Suggested live test: run the incumbent, then sizes 209, 208, 207, 206 and
  finally 205.  Stop the ladder at the first failure or displayed-speed
  regression and retain the last successful form.
- Result: _not yet tested locally in the game_.

### [ ] Year 15 - Shred Lines - community size 6 📋

- **Paste-ready program:** [SolutionsToTry/Year 15 - Shred Lines - community size 6.txt](<SolutionsToTry/Year 15 - Shred Lines - community size 6.txt>)
- Goal: locally confirm abfipes12's public size-6 program, imported to
  Solutions50+ below our size-8 main row (found in the 2026-08-17 source
  audit; it was never in our tables).
- Public evidence: abfipes12 reports 16/25 real-game wins (64%) at about
  950 seconds.  Those wins were the anchor for the Year 15 machine
  rules, settled in the game on 2026-09-06: a step aimed at a shredder
  is a fence (the worker stays put), and an empty-handed give at a
  shredder hands the worker over.  Our own Year 15 four and five, which
  relied on the old model, failed live that day and are archived in
  REJECTED_APPROACHES.md; this six is now the level's smallest published
  program.
- Measured here under the machine rules: **82.5%** at the live cap
  (95.5% before them), about 54,600 frames; re-measured 2026-10-18 at
  163/200 plain and 175/200 shuffled, and again after the same day's
  review corrections at 162/200 plain and 165/200 shuffled, about 58,600
  frames.
- Mechanism: a random seven-direction walk with a guarded pickup/give; the
  give lands on the south shredder row.  No `myitem` anywhere, so the
  refuted Year 15 gated-form class does not apply.
- Expected editor size: **6**; paste-only (multi-direction random step).
- The Solutions50+ row already rests on the public evidence, so a local
  confirmation is optional — after the size candidates above.  A
  mutation search for a five under the machine rules found nothing.
- Result: _not tested locally; the row stands on public evidence_.

### [ ] Year 38 - Seek and Destroy 3 - community speed 6-7 at size 122

- **Paste-ready program:** [SolutionsToTry/Year 38 - Seek and Destroy 3 - community speed 6-7.txt](<SolutionsToTry/Year 38 - Seek and Destroy 3 - community speed 6-7.txt>)
- Goal: locally confirm abfipes12 and commonnickname's public speed
  program, imported to Solutions50+ below our 9-10 main speed row
  (found in the 2026-08-17 source audit; it was never in our tables).
- Public evidence: 67/125 real-game wins (53.6%) at displayed speed 6-7.
- Local emulator: 669/1000 plain and shuffled (2026-10-18; 36/50
  before), average 429 frames (win/fail evidence only; displayed speed is
  async wall-time and theirs is live-measured).
- Expected editor size: **122**; glitchless, so it should also be
  typable/editable normally.
- Suggested live test: repeated runs until a win; capture displayed speed
  and editor size.
- Result: _not yet tested locally in the game_.

### [ ] Year 30 - Fill the Floor - probabilistic size 4 📋

- **Paste-ready program:** [SolutionsToTry/Year 30 - Fill the Floor - probabilistic size 4.txt](<SolutionsToTry/Year 30 - Fill the Floor - probabilistic size 4.txt>)
- Goal: live-confirm the size-4 Solutions50+ row (already published at
  ~1211) below the size-5 main entry.
- The eight-sided four that superseded it on 2026-09-14 was withdrawn on
  2026-10-18 (see REJECTED_APPROACHES.md: never paste it), so this is the
  Year 30 four to run.
- Machine-reach note: this program takes from printers diagonally
  (`takefrom nw,sw,ne`), and its 15/25 public wins show that a diagonal
  printer take is safe, unlike the diagonal shredder give that killed
  Year 21's givers.
- Public evidence: abfipes12 and martinez8859 report 15/25 wins (60%).
- Current emulator evidence: 147/200 plain and 149/200 shuffled
  (2026-10-18; 618/1000 and 616/1000 on 2026-09-14).  Winning runs average
  about 1,126 s, so they often finish only just before the game
  deadline.
- Expected editor size: **4**; paste-only because of the multi-direction
  `takefrom`.
- Suggested live test: 10 uninterrupted runs, allowing every run to reach the
  game's own deadline.
- Result: _not yet tested locally in the game_.

### [ ] Year 30 - Fill the Floor - alternate size 5

- **Paste-ready program:** [SolutionsToTry/Year 30 - Fill the Floor - alternate size 5.txt](<SolutionsToTry/Year 30 - Fill the Floor - alternate size 5.txt>)
- Goal: tie the size-5 record with a faster typical run.
- Exact edit: in the size-5 record, change the step set
  `nw,w,sw,s,ne,e,se` to `nw,sw,n,e,se`.
- Emulator A/B evidence (frame-derived, so it cannot settle displayed
  speed; the live A/B below decides), 300 worlds at the live clock: plain,
  candidate 299/300 averaging 615 seconds against the incumbent's 299/300
  at 649; shuffled, candidate 299/300 at 619 against 300/300 at 627.  The
  win rates match on both screens, but the edge shrinks from about 33
  seconds to about 8 under the shuffled screen, and the incumbent's actual
  game score is about 588, so the improvement may not carry over.
  Re-measured 2026-10-18 over 200 worlds: plain, candidate 197/200 at 616
  seconds against the incumbent's 200/200 at 647; shuffled, 199/200 at
  641 against 199/200 at 647, an edge of about 6 seconds.
- Suggested live test: five alternating fresh runs of the incumbent and this
  candidate; compare medians and timeout count.
- Result: _not yet tested in the game_.

## Low-percent leads (parked — 50%+ only for now)

Valid candidates below the Solutions50+ bar.  Not for the next session;
kept intact so nothing is rediscovered.

### [ ] Year 44 - Unique Fashion Party - static-cull low-percent size 3

- **Paste-ready program:** [SolutionsToTry/Year 44 - Unique Fashion Party - static-cull low-percent size 3.txt](<SolutionsToTry/Year 44 - Unique Fashion Party - static-cull low-percent size 3.txt>)
- Goal: establish a practical size-**3** SolutionsLowPercent row below the
  public size-4 low-percent and size-5 Solutions99+ entries.
- Mechanism: the three-term static predicate kills 38 workers before pickup
  and leaves exactly seven stable survivors.  Four survivors hold guaranteed
  members of the level's shuffled 0-6 set; the remaining three hold ordinary
  random cubes.  A win occurs when those three supply the missing values.
- Nominal probability: `3! / 7^3 = 6 / 343`, or **1.749271%**.  A faithful
  1,000,000-state model produced 17,405 wins (1.7405%).  The emulator,
  which now deals the level's 0-6 at run start as the game does, gives
  12/1000 plain and 12/1000 shuffled (2026-10-18).
- Expected editor size: **3**; the condition has three terms, below the parser
  limit, and `if`, `calc`, and `pickup` are all available in Year 44.
- Suggested live test: 100-200 fresh attempts at 12x.  On a stable loss, verify
  that exactly seven workers survive; any other survivor count falsifies the
  static classification immediately.
- Result: _not yet tested locally in the game_.

### [ ] Year 06 - Little Exterminator 1 - exact-route low-percent size 5 📋

- **Paste-ready program:** [SolutionsToTry/Year 06 - Little Exterminator 1 - exact-route low-percent size 5.txt](<SolutionsToTry/Year 06 - Little Exterminator 1 - exact-route low-percent size 5.txt>)
- Goal: establish a practical size-**5** SolutionsLowPercent row below the
  published size-7 low-percent and size-8 main entries.
- Mechanism: six required binary moves reach the lower funnel with probability
  1/64; seven of the eight three-step tails then reach a position whose west
  pickup takes the cube.  Every earlier deviation falls into a hole.
- Exact density over the level's random start states:
  **1.367187500318%**.  There is one absorbing losing tail at `(7,10)`.
- Expected editor size: **5**; the label is free and the program has one
  pickup, three steps, and one jump.  Paste is required for the direction
  lists at this early level.
- The pickup is written `pickup c,w` (2026-10-18): the game refuses a
  pickup naming a single direction other than `c` on Years 2-6, so the
  first form, `pickup w`, would not paste.  The list passes over the
  worker's own bare square and lifts the cube to the west, so the run is
  the same; the model gives the new form 66/5,000 = 1.32% plain and the
  same shuffled, in line with the exact 1.367%; 15/1000 on both
  screens after the 2026-10-18 corrections.
- Suggested live test: repeated fresh attempts at 12x, resetting shortly
  after the longest successful path; do not wait for the 1,400-second cap when
  the worker is visibly parked in the losing corner.
- Local-emulator cross-check (2026-08-22): 36/3,000 = **1.20%**, within one
  sigma of the exact 1.367% claim — the rate is confirmed by a second model.
- Result: _not yet tested locally in the game_.

### [ ] Year 23 - Sorting Hall - low-percent speed tie-break at size 19 (fallback 21)

- **Paste-ready program:** [SolutionsToTry/Year 23 - Sorting Hall - low-percent speed tie-break at size 19.txt](<SolutionsToTry/Year 23 - Sorting Hall - low-percent speed tie-break at size 19.txt>)
- **Fallback (size 21):** [SolutionsToTry/Year 23 - Sorting Hall - low-percent speed fallback at size 21.txt](<SolutionsToTry/Year 23 - Sorting Hall - low-percent speed fallback at size 21.txt>)
- Goal: retain the low-percent speed row's displayed ~14 while reducing its
  size from 23 to **19** (n05ucc4u's program; keep the credit).
- Exact edits: delete the two three-line re-check tails — in the `> 49`
  branch `if w > myitem: jump d / endif` and in the `else` branch
  `if e < myitem: jump h / endif` — and the labels `d:` and `h:` they
  leave unused (the game refuses a paste with a label no jump leads to;
  labels are free, so the sizes stand).  The size-21 fallback deletes only
  the first tail and `d:`.
- Emulator evidence: 300-trial A/B on one model — incumbent 155 wins at
  16.3 modelled seconds; size 21: 135 wins at 16.2; size 19: 114 wins at
  16.2.  The win rate drops from about 52% to about 38-45% (still the
  low-percent tier) with the speed distribution unchanged.  Re-measured
  2026-10-18 over 1000 worlds: incumbent 508, size 21 426, size 19 355,
  plain and shuffled alike, all at about 16 modelled seconds.
- Suggested live test: repeated ~14-second runs until a win; capture the
  displayed speed and editor size.
- Result: _not yet tested in the game_.

### [ ] Year 11 - Injection Sites 1 - low-percent speed tie-break at size 13 📋

- **Paste-ready program:** [SolutionsToTry/Year 11 - Injection Sites 1 - low-percent speed tie-break at size 13.txt](<SolutionsToTry/Year 11 - Injection Sites 1 - low-percent speed tie-break at size 13.txt>)
- Goal: establish a low-percent size-13 program that retains or improves the
  reliable incumbent's displayed speed of 5 and is smaller than its size 16.
- Exact edit: in the current speed program, replace the final five-line
  `if n == nothing: step n; else: step s; endif` with random `step n,s`.
- Capped-emulator evidence: 1/100 candidate runs won in 278 frames with
  modelled speed 5 and 18 item actions.  The incumbent was 100/100 at 311
  frames and 24 actions; canonical sizes are 13 and 16.  Re-measured
  2026-10-18: 17/1000 plain and shuffled at 328 frames, the incumbent
  1000/1000 at 361.
- Construction caveat: random multi-direction movement is paste-only here, so
  retain the clipboard marker and classify the program as LowPercent.
- Suggested live test: repeated quick attempts; on a win, capture both the
  displayed speed and editor size.
- Result: _not yet tested locally in the game_.

### [ ] Year 13 - Injection Sites 2 - low-percent speed tie-break at size 17 📋

- **Paste-ready program:** [SolutionsToTry/Year 13 - Injection Sites 2 - low-percent speed tie-break at size 17.txt](<SolutionsToTry/Year 13 - Injection Sites 2 - low-percent speed tie-break at size 17.txt>)
- Goal: establish a low-percent speed-5 program at size 17 versus the reliable
  incumbent's size 20.
- Exact edit: retain the outer guard, but replace its inner
  `if ne != datacube: step ne; else: step sw; endif` with random `step ne,sw`.
- Capped-emulator A/B evidence: candidate won 23/100.  Every winning run was
  frame-identical to the incumbent at 326 frames and modelled speed 6, while
  using 21 instead of 24 item actions.  Re-measured 2026-10-18: 235/1000
  plain, every win at the incumbent's 370 frames (it wins 1000/1000), and
  175/1000 shuffled, averaging 399; 24 item actions to its 27.  Canonical
  sizes are 17 and 20; the incumbent's authoritative live score is 5.
- Construction caveat: the random diagonal step is paste-only, so retain the
  clipboard marker and LowPercent classification.
- Suggested live test: a handful of attempts should normally produce a win;
  capture the completion panel and editor size.
- Result: _not yet tested locally in the game_.

### [ ] Year 34 - Seek and Destroy 1 - low-percent speed tie-break at size 83

- **Paste-ready program:** [SolutionsToTry/Year 34 - Seek and Destroy 1 - low-percent speed tie-break at size 83.txt](<SolutionsToTry/Year 34 - Seek and Destroy 1 - low-percent speed tie-break at size 83.txt>)
- Goal: retain the current low-percent speed record of about 6 while reducing
  its secondary size from 84 commands to 83.
- Exact edit: start from
  [the current low-percent speed program](<SolutionsLowPercent/Year 34 - Seek and Destroy 1 (speed).txt>)
  and delete the third `mem1 = nearest datacube` in the `mem2 == mem3` branch,
  immediately before `if n <= mem2`.
- Why it should be safe: no path reads that value; after the branch picks up
  either north or `mem2`, every continuation overwrites `mem1` with the nearest
  shredder.  `nearest` itself is timing-free in the validated model.
- Same-seed emulator A/B evidence: candidate and incumbent won the identical
  5/20 seeds, each in exactly 411 frames with modelled speed 7 and 11 item
  actions.  Re-measured 2026-10-18: both 322/1000 plain and shuffled, at
  the same 411 frames and 11 item actions.  Canonical sizes are 83 and 84.
- Expected editor size: **83**; expected displayed speed: **about 6**.
- Suggested live test: run repeated candidate attempts until it wins, then
  capture the completion panel and editor size.
- Result: _not yet tested locally in the game_.

### [ ] Year 44 - Unique Fashion Party - low-percent size 4

- **Paste-ready program:** [SolutionsToTry/Year 44 - Unique Fashion Party - low-percent size 4.txt](<SolutionsToTry/Year 44 - Unique Fashion Party - low-percent size 4.txt>)
- Goal: live-confirm the new size-4 SolutionsLowPercent entry below the
  size-5 main record.
- Public evidence: abfipes12 reports positive real-game wins in the
  [original posting](https://github.com/abfipes12/7-Billion-Humans-Solutions/blob/main/Test%20Solutions/low%25/Year%2044%20-%20Unique%20Fashion%20Party),
  whose header reads "failed 40 out of 50 => 10% successful": 40 failures in
  50 runs would be 20%, so the line is internally inconsistent.
- Current emulator evidence (2026-10-18, now that the model deals the
  level's 0-6 at the start of every run, as the game does): **73/1000
  plain and 92/1000 shuffled**, in line with the live source's "10%".
  It read 0/20 while the model drew those seven cubes independently.
- Expected editor size: **4**.
- Suggested live test: 10-20 runs; capture the final cube/worker arrangement on
  every failure.
- Result: _not yet tested locally in the game_.

### [ ] Year 44 - Unique Fashion Party - transient-survivor size 3

- **Paste-ready program:** [SolutionsToTry/Year 44 - Unique Fashion Party - transient-survivor size 3.txt](<SolutionsToTry/Year 44 - Unique Fashion Party - transient-survivor size 3.txt>)
- Goal: establish a size-3 SolutionsLowPercent record below the public size-4
  low-percent entry and size-5 main entry.
- Mechanism: all 45 workers take their cubes and walk toward the room's holes.
  The goal is checked every frame, so the run wins during any transient frame
  with exactly seven survivors whose held values are a permutation of 0-6;
  the workers do not need to remain stable afterward.
- Constructive evidence: 38 workers can follow finite routes into upper/bottom
  holes within 13 strides while seven designated workers take longer cardinal
  routes, leaving a nonempty exactly-seven window.
- Model evidence (2026-10-18, now that the model deals the level's 0-6 at
  the start of every run, as the game does): **10/1000 plain and 17/1000
  shuffled**, on the 1-in-100 line, so it moved up from the parked long
  shots.  The model now wins the public size-4 program too (73/1000 plain,
  92/1000 shuffled).  The figure first given here, `7! / 7^7` (about
  0.612%) for value uniqueness, assumed independent draws.
- Expected editor size: **3**.
- Suggested live test: repeated fresh runs, capturing every exactly-seven
  survivor pattern and the first completion panel.
- Result: _not yet tested locally in the game_.

### [ ] Year 05 - An Important Decision - absorbing low-percent size 2 📋

- **Paste-ready program:** [SolutionsToTry/Year 05 - An Important Decision - absorbing low-percent size 2.txt](<SolutionsToTry/Year 05 - An Important Decision - absorbing low-percent size 2.txt>)
- Goal: establish a size-2 SolutionsLowPercent record below the existing
  size-4 low-percent entry and the size-5 main entry.
- Mechanism: each of the four workers performs an independent one-dimensional
  random walk until it falls into one of the two row holes.  The level wins
  only when every worker reaches its designated side; under unbiased choices
  the exact success probability is `24 / 2,401`, or about 1.00%.
- Capped-emulator evidence: 12/1,000 wins (1.2%), average winning speed 8.0,
  range 5-14, and winning frames 253-862.  The observed rate agrees with the
  static probability and no winning run approached the deadline.  After the
  2026-10-18 corrections: 2/200 plain and 4/200 shuffled.
- Expected editor size: **2**.
- Entry method: paste the text; random multi-direction `step w,e` is not
  constructible from Year 05's normal editor palette.
- Suggested live test: repeated fresh runs; roughly 300 attempts give about a
  95% chance of seeing at least one win if the real game's direction choices
  are unbiased.  Capture the first completion panel.
- Result: _not yet tested locally in the game_.

### [ ] Year 13 - Injection Sites 2 - recoverable low-percent size 5 📋

- **Paste-ready program:** [SolutionsToTry/Year 13 - Injection Sites 2 - recoverable low-percent size 5.txt](<SolutionsToTry/Year 13 - Injection Sites 2 - recoverable low-percent size 5.txt>)
- Goal: improve the existing size-6 SolutionsLowPercent entry to size 5.
- Provenance: this is H-J-Granger's public low-percent program with only its
  initial `step se` removed; retain that attribution if it is promoted.
- Mechanism: all six workers first take their cubes, then use the same
  recoverable six-direction walk and exact gap predicate as the public
  program.  Removing the initializer creates additional hole-loss paths but
  leaves collision-free successful routes reachable.
- Capped-emulator evidence: 14/100 wins; winning speed averaged 365.9, ranged
  from 59 to 1,223, and used 3,635-76,436 frames.  Re-measured 2026-10-18:
  33/200 plain and 30/200 shuffled, winning runs averaging about 437 s.
- Expected editor size: **5**.
- Entry method: paste the text because the six-direction random step is not
  constructible from the normal editor controls.
- Suggested live test: repeated fresh runs; capture a completion panel and
  confirm editor size 5.  The emulator sample suggests this should be much
  more practical than the rarer one-shot entries below.
- Result: _not yet tested locally in the game_.

### [ ] Year 22 - Number Royale - survivor low-percent size 3 📋

- **Paste-ready program:** [SolutionsToTry/Year 22 - Number Royale - survivor low-percent size 3.txt](<SolutionsToTry/Year 22 - Number Royale - survivor low-percent size 3.txt>)
- Goal: improve the existing size-4 SolutionsLowPercent entry to size 3.
- Mechanism: every worker takes its own cube, then performs an independent
  north/south random walk until falling through the disposal hole.  The level
  wins when all non-maximum holders have died while at least one maximum holder
  remains alive, and the survivors, who walk on for ever, keep holding the
  maximum for more than a second; losing the last maximum cube ends the run.
- Emulator evidence (2026-10-18, with the game's own goal and its deal of
  ten different numbers): 93/1,000 plain and 95/1,000 shuffled, winning runs
  averaging 1,018 frames (about 16 s); 84/1,000 before.  Every win
  completes far before the deadline.
- Expected editor size: **3**.
- Entry method: paste the text because `step n,s` is a random multi-direction
  command unavailable from the normal editor controls.
- Suggested live test: repeated quick runs; verify that the completion panel
  appears while at least one maximum-valued worker is still alive and capture
  editor size 3.
- Result: _not yet tested locally in the game_.

### [ ] Year 54 - Terrain Leveler - constant-average low-percent size 5

- **Paste-ready program:** [SolutionsToTry/Year 54 - Terrain Leveler - constant-average low-percent size 5.txt](<SolutionsToTry/Year 54 - Terrain Leveler - constant-average low-percent size 5.txt>)
- Goal: establish a size-5 SolutionsLowPercent record below the size-9 main
  entry.
- Mechanism: all seven workers sweep straight north through their columns,
  rewriting every cube to 3.  The run wins exactly in worlds whose original
  49-cube average rounds down to 3.
- Probability analysis: accounting for the level's random 0-6, 0-10, and 0-20
  range modes gives an intended-world probability about 0.259.  The 0-6 mode
  alone has probability about 0.514 of averaging to 3.
- Capped-emulator evidence: 20/100 wins; winning speed averaged 30.6, ranged
  from 29 to 31, and used 1,786-1,936 frames.  Re-measured 2026-10-18:
  49/200 plain and shuffled, winning runs at about 1,910 frames.
- Expected editor size: **5**; all commands are available in Year 54.
- Suggested live test: repeated quick runs; record the random range and capture
  the first completion panel with editor size 5.
- Result: _not yet tested locally in the game_.

## Parked long shots (win rate below 1 in 100)

Per the maintainer's rule, candidates that win less often than **1 in
100 runs** are not part of the live-game queue: a witnessed completion
would cost more restart-grinding than the row is worth.  They remain
here (with their paste-ready files) in case the rule changes or a
higher-rate variant is found.  Nothing below this line needs game time.

### [ ] Year 44 - Unique Fashion Party - divide-by-zero long-shot size 2

- **Paste-ready program:** [SolutionsToTry/Year 44 - Unique Fashion Party - divide-by-zero long-shot size 2.txt](<SolutionsToTry/Year 44 - Unique Fashion Party - divide-by-zero long-shot size 2.txt>)
- Goal: preserve the absolute size-**2** construction below the practical
  size-3 candidate.  It is parked because its rate is far below the 1% live
  queue threshold.
- Mechanism: `pickup n` leaves 35 cube-holders.  After those pickups, exactly
  ten still see an unpicked cube south; everyone else divides by zero and
  dies.  A win occurs when exactly seven south denominators are nonzero and
  the corresponding seven held labels are the complete set 0-6.
- Probability evidence: 131/200,000 faithful modeled states won (0.0655%);
  the iid calculation is 0.072778626%, and a concrete winning start state
  exists in the model.  Both figures assume the seven cubes are drawn
  independently; the game deals them 0-6 once each, and the corrected
  emulator gives 0/1000 plain and shuffled (2026-10-18).
- Expected editor size: **2**.  Size 1 cannot both acquire cubes and remove
  redundant workers.
- Suggested live test: none under the current cutoff; retain for a future
  reproducible RNG harness or a lucky natural completion.
- Result: _not yet tested locally in the game_.

### [ ] Year 12 - Unzip - one-shot low-percent size 3 📋

- **Paste-ready program:** [SolutionsToTry/Year 12 - Unzip - one-shot low-percent size 3.txt](<SolutionsToTry/Year 12 - Unzip - one-shot low-percent size 3.txt>)
- Goal: establish a size-3 SolutionsLowPercent record below the size-5 main
  entry.
- Mechanism: all 12 workers pick up their on-tile cubes, independently choose
  north or south once, and drop.  Exactly one of the `2^12 = 4,096` direction
  patterns is the required alternating zipper.
- Capped-emulator evidence: 23/100,000 wins, close to the theoretical 1/4,096
  rate; every win completed in exactly 56 frames with displayed speed 1.
- Expected editor size: **3**; paste-only because of `step n,s`.
- Suggested live test: use repeated fresh runs rather than waiting within one
  run—the program finishes immediately.  A live win may take several thousand
  attempts, so this is lower priority than the main-tier candidates.
- Result: _not yet tested locally in the game_.

### [ ] Year 52 - The Mode Code - one-shot low-percent size 6

- **Paste-ready program:** [SolutionsToTry/Year 52 - The Mode Code - one-shot low-percent size 6.txt](<SolutionsToTry/Year 52 - The Mode Code - one-shot low-percent size 6.txt>)
- Goal: establish a size-6 SolutionsLowPercent record below the size-15 main
  entry.
- Mechanism: each worker binds its own result cube, samples the input cube two
  rows north, and writes that sample plus 8.  The level wins exactly when the
  six sampled values happen to equal the six true frequency counts minus 8.
- Exact probability: conditioning on the six sampled cubes and the remaining
  58 independent uniform draws gives `2.90709234823e-6`, or about one win in
  343,986 worlds.
- Witness: an independent exact search of the model's random worlds
  predicted the first winning world at seed 69,510 with counts `[13,9,11,11,12,8]` and samples
  `[5,1,3,3,4,0]`.  The capped emulator then found exactly 1/69,510 wins, at
  that final seed, completing in 342 frames with displayed speed 6.
- Expected editor size: **6**; every command is available in Year 52.
- Suggested live test: this is mathematically sound but far too rare for a
  practical manual campaign.  Preserve it for a lucky natural run or a future
  reproducible live-game RNG harness; capture the completion panel if tested.
- Result: _not yet tested locally in the game_.

### [ ] Year 55 - Data Flowers - constant-sum low-percent size 5 📋

- **Paste-ready program:** [SolutionsToTry/Year 55 - Data Flowers - constant-sum low-percent size 5.txt](<SolutionsToTry/Year 55 - Data Flowers - constant-sum low-percent size 5.txt>)
- Goal: establish a size-5 SolutionsLowPercent record below the size-7 main
  entry.
- Mechanism: the five workers march north through their flower centers, move
  one south petal into each center, and write the constant 36.  The level wins
  exactly when every independent eight-value flower ring originally sums to
  36; moving a petal does not change the stored target sum.
- Exact probability: one eight-value ring sums to 36 with probability
  `4,816,030 / 10^8`; all five do so with probability
  `2.5908717630610564e-7`, or about one in 3,859,705.
- Witness: an independent search of the model's random worlds found seed
  3,868,438.  All five eight-value groups sum to 36, and the isolated seed-offset emulator
  won in 2,099 frames with displayed speed 34 and 172 item actions.
- Expected editor size: **5**.
- Entry method: paste the text because the ordered multi-target `pickup c,s`
  is not constructible from Year 55's normal editor controls.
- `pickup c,s` passes over the bare centre to the petal, as the game's
  lists do (Year 04's `pickup c,e` record relies on the same).
- Suggested live test: natural manual verification is impractical without a
  reproducible RNG-start method.  Capture the completion panel if the matching
  world can be reproduced.
- Result: _not yet tested locally in the game_.

### [ ] Year 56 - Local Maximums - one-shot low-percent size 3

- **Paste-ready program:** [SolutionsToTry/Year 56 - Local Maximums - one-shot low-percent size 3.txt](<SolutionsToTry/Year 56 - Local Maximums - one-shot low-percent size 3.txt>)
- Goal: improve the existing size-4 SolutionsLowPercent entry and size-7 main
  entry to size 3.
- Mechanism: each worker takes the northwest cube from its own eight-cube
  group and feeds its nearest shredder.  The level wins exactly when every
  shredded number is the largest number some group started with, its own
  or another's, so the omitted `write 99` is unnecessary in that world.
- Exact probability (corrected 2026-10-16): `2.1092691716e-6`, or about
  one in 474,098.  The figure first given here, `6.2945867338e-7` (one in
  1,588,667), counted only the worlds where every shredded cube is its own
  group's maximum.
- Witness: an independent search of the model's random worlds found seed
  3,281,406.  Its selected values are `[70,87,83,79,96,98,90]`, each the maximum of its
  group.  An isolated seed-offset emulator run then won in 310 frames with
  displayed speed 5 and 14 item actions.
- Expected editor size: **3**; all commands are available in Year 56.
- Suggested live test: preserve this as a mathematically witnessed rare record;
  natural manual verification is impractical without a reproducible RNG-start
  method.  Capture the completion panel if the matching world occurs.
- Result: _not yet tested locally in the game_.

### [ ] Year 62 - The Sorting Floor - initially sorted size 0

- **Paste-ready program:** [SolutionsToTry/Year 62 - The Sorting Floor - initially sorted size 0.txt](<SolutionsToTry/Year 62 - The Sorting Floor - initially sorted size 0.txt>)
- Goal: establish a zero-command SolutionsLowPercent record below the size-10
  Solutions50+ and size-12 main entries.
- Mechanism: do nothing.  The nine independent random cubes occasionally
  spawn in weakly increasing row-major order, satisfying the goal before the
  first frame is processed.
- Exact probability: `C(108, 9) / 100^9 = 3.9113958819e-6`, or about one win
  in 255,663 worlds.
- Witness: an independent search of the model's random worlds found seed
  239,189, whose values in row-major order are
  `[10,14,18,41,62,69,80,88,95]`; the isolated seed-offset emulator accepted
  the empty program at frame 0 with size 0 and displayed speed 0.
- Expected editor size: **0**.  The paste file holds only the header lines,
  so pasting it (or clearing the program by hand) leaves nothing to run; a
  lone label would not do, since the game refuses a label no jump leads to.
- Suggested live test: natural manual verification is impractical.  If a
  reproducible live-game RNG-start method becomes available, clear the
  program, run the matching world, and capture the immediate completion
  panel.
- Result: _not yet tested locally in the game_.

### [ ] Year 33 - Data Backup Day - one-shot low-percent size 5

- **Paste-ready program:** [SolutionsToTry/Year 33 - Data Backup Day - one-shot low-percent size 5.txt](<SolutionsToTry/Year 33 - Data Backup Day - one-shot low-percent size 5.txt>)
- Goal: establish a size-5 SolutionsLowPercent record below the size-7 main
  entry.
- Mechanism: every worker remembers its east value, selects one of its two
  equidistant cubes, and overwrites the selected cube.  The correct nearest
  tie choice in all eight pairs has probability close to `1/2^8`.
- Capped-emulator evidence: 44/10,000 wins (0.44%); every win completed in 146
  frames with displayed speed 3.
- Expected editor size: **5**; all commands are available in Year 33.
- Suggested live test: repeated quick runs; roughly a few hundred attempts per
  observed win is plausible, but record the actual tie behavior.
- Result: _not yet tested locally in the game_.
