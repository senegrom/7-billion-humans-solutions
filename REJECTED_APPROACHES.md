# Rejected and Superseded Approaches

These experiments are kept so failed ideas are not rediscovered and mistaken
for records.  Candidates that still need live-game verification remain in
[SOLUTIONS_TO_TRY.md](SOLUTIONS_TO_TRY.md).

- Y24 size 3: emulator false positive; real handoff semantics do not sustain it.
- Y15 size-5 give-first ordering: 300/300, but average and worst-case timing
  were slightly worse than the then-preferred pickup-first ordering on the
  same seeds (191.4 vs 190.9 average; 387 vs 378 worst).  That pickup-first
  event-gated program was itself later refuted paste-verbatim in the live game.
- Y15 symmetric size-5 gate `(shredder and holding) or north-cube`: 300/300,
  but much slower than the then-preferred empty-worker gate (265.5 average,
  727 worst).  The empty-worker form was later refuted live too, so neither
  one-IF family is retained.
- Y15 community inner-guard deletion (size 6→5): removing only the inner
  `if nw == datacube or s == shredder` around `pickup nw` looked state-neutral
  because every newly attempted pickup points at a known non-cube, but it
  failed 0/20 fixed-order worlds while abfipes12's size-6 control won the
  identical 20/20 (59,848.7 frames, 5,031.4 items).  Failed-pickup timing is
  load-bearing on Shred Lines; no shuffled test was run.
- Y16 size-4 random `step s,e` route: a finite three-cube path exists, but two
  buffered witness batches timed out without yielding a result and the static
  route probability is too low for a practical live test.  Retain as theory,
  not as a queue entry.
- Y10 random-walk size 2: 0/100 capped emulator wins; any winning path is too
  rare to justify live-game queue space without stronger evidence.
- Y10 speed size-32 branch fusion: it still wins, but takes 867 model frames
  versus the incumbent's 790 and is therefore not a speed-record tie-break.
- Y07 deterministic size-9 pickup linearization: it wins, but adds a 93-frame
  invalid-pickup bubble and slows modelled speed from 4 to 5.  The then-leading
  error-free size-11 candidate was later refuted live as timing-sensitive, so
  neither candidate is retained.
- Y12 speed size-16 random final-direction fusion: 0/100 while the incumbent
  was 100/100 in 197 frames; endpoint motion changes later branch entrants.
- Y14 pickup-first size 4: the existing live size-4 and this permutation both
  retain the cube-less worker's failed pickup/give path.  Disassembly shows no
  action-animation saving, and the corrected emulator makes them exactly tied
  at 97 frames even though it misreports the incumbent's live timing.  It is
  therefore expected to retain the live score of 4, not challenge speed 2.
- Y18 random-fan size 4: 0/100 capped emulator wins; the workers do not fan out
  to ten unique shredders reliably enough to retain as a live candidate.
- Y22 public size 4: its header says 86 failures out of 150, which is 42.7%
  success rather than the claimed 57.3%; current emulator is 48/100.  It ties
  the existing LowPercent size-4 program while being far slower, so it is
  dominated rather than a Solutions50+ candidate.
- Y22 synthesized size-3 `step all; pickup c,w; jump` program: 29/1,200 wins
  (2.4%) at 116 seconds average.  It ties the retained survivor program's size
  3 but is dominated by that candidate's 84/1,000 wins and 14.4-second average.
- Y23 speed size-8 deletion of the west branch's immediate `jump a`: 100/100
  wins, but average completion worsens from 1,059.0 to 1,382.3 frames and from
  modelled speed 17.5 to 22.7.  It is also larger than the current size-6
  endpoint, so it improves neither record.
- Y23 if/else priority loop at canonical size 7 (emulator size 6): correct but
  dominated.  Against the promoted size-8 speed row on the same 100 fixed-order
  worlds, both won 100/100, but the rewrite regressed average frames
  1,091.0→1,408.2, modelled speed 17.9→23.0 (range 12–22→15–28), and item
  actions 491.3→646.1.  It therefore challenges neither the 8/15–17 speed row
  nor the canonical size-6/23–25 size row.
- Y25 unconditional-give size 8: it completed correctly at canonical size 8,
  but averaged 134 seconds live versus 130 seconds for the correct size-9
  incumbent.  The level's size-row best is 5, so it improves neither record
  and is dominated.
- Y25 external persistent-loop reduction: the source program is 32 commands at
  129 seconds and the proposed unconditional-tail form is about 30 commands at
  the same speed.  Both are dominated by the local size-9/live-129 endpoint,
  and neither challenges the size-5 row.
- Y26 size 6: invalid for the split-50 goal; the old loader ignored that rule.
- Y26 size-5 ordered take/give fusion: three distinct routing predicates each
  failed 0/100 at the 1,400-second cap; the mechanism stalls the feeder chain.
- Y20 speed size-36 ordered-pickup fusion: 0/1 while the incumbent won; the
  removed full-hands pickup error was a required synchronization delay.
- Y30 speed first branch-terminal `end` deletion: 0/1 at the real cutoff;
  without it that worker falls into the common-tail work before the exact fill
  goal is complete.
- Y31 unconditional size 5: 0/100; removing the parity/obstacle guard causes
  irreversible wrong-square drops rather than an absorbing checkerboard fill.
- Y31 diagonal size-4 families: `takefrom` failed 0/10 because workers steal
  sheets from adjacent hands and break the parity invariant.  Replacing it with
  `pickup` preserves parity, but a corrected printer/floor-relay model was
  0/1,000 because workers repeatedly remove already-filled goal cubes.
- Y33 ordered-pickup size 4: 0/1,000.  Directional `pickup w,e` does not move
  the worker onto the source tile, so terminal `drop` restores the copy on the
  center square; the extra restore step in the queued size-5 form is essential.
- Y34 alternate size 7 safe-init rewrite: 29/100 at the real cutoff.
- Y36 size 7: eventual emulator wins take up to roughly 6,400 seconds, beyond
  the game's 1,400-second limit.
- Y36 deletion of `comment 1` from the size-211 speed candidate: runtime-neutral
  in the model, but `comment N` commands are free in the editor counter, so the
  candidate remains size 211 rather than 210.
- Y37 constant-23 size 11: an exact RNG world with all seven row sums equal to
  23 still fails because removing the repeated `calc` changes worker timing.
  The related size-5 random walk has a finite abstract route but no seed shown
  to realize both the required values and movement schedule.
- Y37 larger state-machine normalizations: canonical size 238 still wins, but
  regresses from 747 frames/modelled speed 12/68 items to 905/15/75.  The
  independently tested size-235 normalization fails 0/1 at the full cap, so
  the combined size-230 form was not attempted.
- Y37 shared 12/14-state increment at size 242: it wins, but moving the state
  update out of the branch changes coordination and regresses from 747
  frames/modelled speed 12/68 items to 930/15/74.  Retain size 243.
- Y38 speed deletion of one, two, or all three consecutive `pickup c` retries:
  94/100, 93/100, and 91/100 respectively, versus 100/100 for the incumbent.
  The actions are reliability choreography rather than removable dead code.
- Y38 Solutions50+ speed size-120 neutral-walk family: both two-step cuts fail
  the item/choreography gate before jitter.  The first removes the opening
  `step w` and the post-ascent `step e`; over 100 paired worlds the size-122
  control won 73 and the candidate 71, with seed 20 changing from a 458-frame
  control win to a full-cap loss.  The distinct adjacent `step e` / `step w`
  cut preserves the whole ascent and pickup square, but reproduced the same
  decisive shared-seed regressions: seed 5 changed 18→19 item actions and seed
  12 changed 17→18 while slowing 382→460 frames.  It was stopped there.
- Y39 unguarded-printer size 5: 0/100 at a 20,000-frame diagnostic cap; random
  requeueing did not produce the required exactly-five-sheets-per-worker state.
- Y39 end-only speed size 172: initially superseded in the emulator by the
  size-167 composition, which included the same terminal-`end` deletion plus
  five nominally neutral diagonals at exactly 2,581 frames.  Size 167 was later
  refuted live at 41 seconds versus 36; the end-only form was not independently
  live-tested and is not retained on frame identity alone.
- Y40 speed branch-`b` and branch-`c` terminal-`end` deletions: each failed
  0/1 at the real cutoff, while the incumbent wins in 2,777 frames.  Both
  workers fall through into later branches before the global goal is complete,
  so neither `end` is post-goal dead code.
- Y40 size-170 PR/diagonal and size-175 PR/dead-calc candidates: initially
  superseded in the emulator by the size-169 composition at exactly 2,697
  frames/100 item actions.  Live testing later refuted size 169 at 41 seconds
  and size 175 at 38, both behind the published size-176 PR form at 36; none of
  these smaller compositions is retained.
- Y41 random-exit size 6: a finite all-west-while-carrying then north-to-hole
  route exists, but even the relaxed geometry estimate is only about 1e-88;
  there is no practical witness search or live test.
- Y42 size-8 loop fusions: both the outer-delivery and unified random-walk
  variants failed 0/5; the separate search and delivery drifts are material.
- Y44 speed size-16 write-only-store deletion: `mem4 = set myitem` is globally
  unread, but removing it fails the exact-action gate.  In 100 fixed paired
  worlds both programs hit the known-unfaithful model's same 163-frame loss,
  while every seed changed 97→95 item actions because the store executes twice
  across the crew.  No shuffled or live test was run.
- Y48 four-command speed rewrite: emulator training-goal false positive; the
  instructor does not shred its demonstration cube.
- Y48 size-3 `takefrom n,s` fusion: behaviorally collapses to the previously
  reverted direct-south size-3 program and removes the incumbent's
  role-dependent opening look/error timing.  The aggregate emulator accepts
  that shortcut, but the live game already rejected it; retain size 4.
- Y49 size-3 deletion of the final east clear step: 0/1 while the size-4
  incumbent won in 530 frames; the post-feed clear is required for the later
  shredder queue.
- Y54 deletion of either duplicated `mem2 = set c` before `tell everyone hi`:
  0/1 while the incumbent won in 2,673 frames.  The overwritten value is dead,
  but the store is a required tell/listen synchronization delay.
- Y54 uniform-average random-walk size 5: correct at 49/200, but superseded
  before live testing by the existing straight-column constant-average size-5
  program.  Both win when the initial integer average is 3; the retained form
  finishes in about 30 seconds rather than roughly 450 and is much safer under
  the 1,400-second cutoff.
- Y54 Solutions50+ speed size-36 deletion: withdrawn under the item-action
  rule.  It deletes a `write 3`, so it necessarily changes item choreography;
  its earlier evidence was also unpaired (50/200 candidate versus 32/150
  incumbent) and it never received the comparative jitter screen.
- Y58 size-3 random, nearest, cardinal-pick, diagonal-pick, and one-shot
  pruning families produced no witness across their bounded screens.  Exact
  graph optimization shows a win must remove at least 12 interior cubes and
  at most 8 perimeter cubes; the tested policies have the wrong removal bias.
- Y60 alternate size 9: superseded by dmr's public 0/200-failure size-9 record.
- Y62 speed empty-inner-loop size 215: 35/100 fixed-order wins versus the
  incumbent's 36/100; its win set loses seed 97, which changes from a
  666-frame/33-item win to a capped 26-item failure.  Choreography is not
  preserved, so it did not advance to the jitter screen.
- Y62 speed both-empty-loops size 214: collapsed to 7/100 fixed-order wins
  (seeds 8, 11, 13, 34, 55, 70 and 99) versus the incumbent's 36/100, and
  item counts differed on every common win.  Materially worse; no jitter.
- Y63 size 9 rewrite: 0/10.
- Y64 size 7 rewrite: 0/5.
- Y64 speed size-99 deletion of one initial `mem1 = set myitem` cadence pad:
  0/1 at the real cutoff while the untouched size-100 incumbent won in 561
  frames.  Even one apparently dead store is phase-critical on this level.
- Y65 size 13 rewrite: 0/52.
- Y66 further padding cuts: deleting one store from the later 14-store block
  fails; deleting the earlier singleton after its first shuffle wins but slows
  from 1,552 to 1,574 frames/modelled speed 26.  Size 240 is the exact-timing
  fallback, but both size 240 and 239 later failed live.
- Y66 endpoint-preserving step-pair reduction at size 244: withdrawn before
  live testing because it fails the comparative jitter gate.  It won only
  29/100 shuffled-dispatch worlds against the incumbent's 45/100, a material
  reliability regression on a press level whose earlier timing cuts failed
  live.
- Y68 unconditional size 5: 0/20; removing the side-hole guard did not produce
  an observable low-percent win.
- Y68 bypassed-wrapper speed size 164: REFUTED LIVE; removing the nominally
  bypassed `foreachdir` wrappers changed real-game execution.  It no longer
  dominates the independent tell-only 171/170 ladder, which remains queued.
  Its latest session was inconclusive because the size-172 control itself
  failed all three attempts.
- Y21 speed four-site reach-merge (step s + takefrom e -> takefrom se at four
  sites of the pre-import program): each site 200/200 alone, combined average
  23.4 s vs 24.9 s over 400 paired trials.  Superseded before live testing:
  the imported community program (@commonnickname + @abfipes12, recorded
  16-22) replaced the base file, and the two programs measure identically in
  the simulator (avg 24.9, same distribution) — the community's real-game
  gain lives in machine-serve timing the simulator does not yet model, so a
  simulator-relative -1.5 s no longer supports an improvement claim against
  the new record.  Re-running the merge transform on the imported bases
  instead.
- Y21 one-walk imported-record reach merge at editor size 40: 400/400 and faster
  than the size-41 base, but initially superseded by the position-preserving
  size-39 fusion, which removed both walks and won 100/100 with a better model
  timing range.  That size-39 fusion was later refuted live at under 10%, so
  neither reach-merge form is retained.
- Y22 size-3 scatter-tumble family (`a: step <list> / pickup <targets> /
  jump a`): rate-optimized across 45 pickup-target and 50 step-list
  variants; the best, `step n,s / pickup c,s`, measures 120/1,200 = 10.0%
  with winning runs averaging 94.5 s (fastest 32 s).  Superseded by the
  hole-dive size 3 in the live queue: despite the lower 8.4% rate, its
  wins land in 5-34 s and its losses self-terminate, so expected
  wall-clock per witnessed completion is far lower.  KEPT AS FALLBACK if
  the hole-dive's live rate disappoints.
- Y58 size-4 top-tier promotion: the 50+ entry measures 384/400 = 96.0%
  at the real 1,400-second clock, losses all clock-outs (slowest win
  1,379 s).  Both improvement axes are exhausted: eleven step-list
  variants (the published nw,w,sw,n,s,ne,e,se order is best, 146/150;
  canonical order 140, cardinals-only 136) and twelve pick-guard
  variants (the published sw/e/se nothing-triple is best, 146/150; every
  relaxation fires wrong picks and lowers the rate).  The program is
  locally optimal at ~96% -- correctly tiered in Solutions50+; no 99+
  claim exists in this neighborhood.
- Y21 speed position-preserving reach merge (final inner-loop `step w;
  giveto s; step e; takefrom s` -> `giveto sw; takefrom s`, size 41 -> 39):
  REFUTED LIVE by the maintainer -- "most of the times the humans get
  shredded; if they don't get shredded it works but very rarely; < 10%".
  The simulator endorsed it 100/100 with the giver never moving, because
  its model lets a give that BEGINS within diagonal reach toss from where
  it stands.  The game instead walks the giver in to deliver and the
  shredder destroys the worker; the rare clean runs fit a frame race.
  RULE CORRECTION this refutation establishes: diagonal reach is fine for
  worker-to-worker gives (the published Year 19 relay does exactly that),
  but a SHREDDER give serves cardinally/at the machine front -- a diagonal
  shredder give is a walk-in death.  Do not propose diagonal machine-give
  merges again; audit any future merge for machine-adjacent geometry.
  The step the merge removed exists precisely to make the give cardinal,
  so no smaller variant of this loop survives the rule.
- Y24 size-4 one-sided relay (`if myitem == something or w == hole:
  takefrom s; giveto e,s`): REFUTED LIVE by the maintainer -- "immediately
  throws the cube back in the printer and then dies (not allowed)".  The
  simulator's ordered give-list treats the south printer square as a
  non-recipient and falls through cleanly (100/100); the game ACCEPTS the
  printer as a give target, returns the sheet into it, and rules the run
  illegal.  Same defect family as the Y21 diagonal-shredder-give death
  and the long-standing Y24 3-command false win: DIRECTIONAL-GIVE TARGET
  SEMANTICS AT MACHINES are wrong in the simulator (it politely skips
  targets the game happily mis-serves, and vice versa).  Until that
  subsystem is read out of a live witness, no candidate whose safety
  argument depends on a give-list falling through a machine square, or on
  any non-cardinal machine give, should be queued.
- THE FRAME-IDENTICAL CLAIM CLASS AND THE DIAGONAL TRANSFORM, refuted
  live on Years 39 and 40 (maintainer A/Bs, same machine, same session):
  Y39 diagonal-stack+end-drop (size 167) ran 41 s against the incumbent's
  36; Y40 PR+dead-calc+six-diagonals (size 169) ran 41 s against the PR
  author's 36.  Both candidates were frame-for-frame identical (or
  faster) in the simulator over hundreds-to-1,000 trials.  Conclusion:
  the displayed speed is ASYNCHRONOUS WALL-TIME — frame identity does not
  establish it, and collapsing two cardinal steps into one diagonal costs
  live wall-time (~1 s per site per pass; machine-queue phase alignment
  suspected).  Diagonalization is dead as a speed/tie-break transform;
  the earlier Y38/Y59 diagonal-stack variants were withdrawn untested as
  the same class.  The currently queued Y38 size-140 and Y59 size-142
  programs instead delete cancelling opposite steps and remain separate
  live-control experiments.  Y40 falls back to the PR-92 ladder (176
  live-verified 36, then 175).  Frame-based speed evidence anywhere in the
  queue is downgraded to "requires live incumbent control first".
- Y40 size-175 (PR-92 plus the data-dead `mem2 = calc [blank] + [blank]`
  deletion): REFUTED LIVE at 38 s versus the PR form's 36 s, despite
  frame-identical simulator evidence.  With the Y39/Y40 diagonal results
  this completes the picture: EVEN PURE DELETIONS of data-dead commands
  shift the asynchronous wall clock — a dead store or calc can be
  load-bearing timing (the pad keeps a worker's arrival in phase with a
  machine's serve rhythm; removing it trades ~250 ms of command time for
  a longer queue wait).  The live ladder protocol (test one edit at a
  time, keep the last non-regressing rung) is therefore mandatory for
  every speed-row size reduction; the simulator cannot pre-clear them.
- Y15 event-gated size 5 (random `step n,s` + guarded pickup/give):
  REFUTED LIVE by the maintainer -- "again all workers die".  The random
  walk steps workers onto the shredder row, and STEPPING ONTO A SHREDDER
  TILE IS DEATH in the game; the simulator had refused the move instead
  (300/300 false).  The rule is now modelled (walkable shredders +
  fatal arrival): the candidate measures 0/100 and the published size-8
  stays 100/100.  Any candidate whose random step list can reach a
  square adjacent to a shredder is in this class -- audit step lists
  against machine adjacency before queueing.
- Y15 stochastic size 4 (random `step n,s` walk family): retracted with
  the event-gated size 5 -- the live report "again all workers die"
  refutes the whole random-step-on-Shred-Lines class (the map's only
  death is the shredder row, so the crew walked into the blades).  Under
  EMU_STEPDEATH=1 screening both members measure 0/50; the published
  size-8 stays 100/100 in either mode.  DISCRIMINATING EXPERIMENTS still
  wanted from the live game: on Year 15, paste (a) `a:/step s/jump a`
  and (b) `a:/step n,s/jump a` for one run each -- whether the crew dies
  or fences against the shredder row under a SINGLE-direction step vs a
  RANDOM step decides the true rule, because several verified solutions
  elsewhere appear to lean on machine rows as fences.  The Y21 diagonal
  give death and the Y24 printer-return death likewise await one-run
  discriminators (their unconditional versions each break verified
  community rows: Years 19/21-size feed diagonally; Years 16/18/42
  deliver past machine squares in list order).
- Y60 ordered-pickup size 8: REFUTED LIVE by the maintainer -- the
  leftmost field stays empty.  The merge dropped the leftmost worker's
  per-lap 1.5 s bubble (its west is always the wall) or fell to the
  game's true pickup-list choice rule; either way the merged form fails.
  REPAIR SWEEP EXHAUSTED with a validated harness (the unmodified
  control wins 30/30 from the same template): every single-command
  deletion that keeps the incumbent's separate pickups loses 0/300, and
  the reordered list `pickup se,w` also loses 0/300 even under the
  simulator's own first-found semantics.  No size-8 exists in this
  neighborhood; the size-9 record stands.  Optional live diagnostic if
  ever curious: run the `pickup se,w` variant once -- identical leftmost
  starvation would indicate the game picks randomly among listed squares,
  while a totally different failure supports ordered first-found.
- CONTAMINATED-RUN AUDIT (maintainer insight): `myitem` conditions cannot
  be TYPED before Year 22, so live tests that retyped programs with
  substitutions tested DIFFERENT programs.  Three refutations reopened as
  paste-verbatim retests: Y15 event-gated size 5, Y24 one-sided relay
  size 4, and Y15 deterministic size 7 (its "runs forever" came from a
  confirmed `c == nothing` substitution).  Supporting evidence: Year 42's
  verified record random-walks a shreddered level live, so random steps
  onto shredders appear fenced — removing the only death mechanism the
  Y15 map offered and undermining the step-death reading of the original
  "all workers die" report.  The step-death stays available as opt-in
  screening until the 30-second single-step discriminator settles it.
  Refutations that STAND (paste-verified or no untypable constructs):
  Y21 reach-merge (diagonal shredder give), Y60 ordered-pickup (leftmost
  starvation; repair space exhausted), Y39/Y40 diagonal stacks and the
  Y40 dead-calc (async wall-clock).
- LIVE SWEEP VERDICTS (maintainer session, eleven results):
  - Y15 event-gated size 5 and Y24 one-sided relay size 4: retested
    PASTE-VERBATIM and still fail the same ways.  The contamination
    theory is dead for both; the original refutations stand FINAL.
    (The Y42-random-walk fence contradiction remains an open mechanic
    question, but no Y15 candidate survives it either way.)
  - Y47 size 33: fails live.  Mechanism: the deleted always-true
    `if myitem == myitem` was load-bearing WALL-TIME -- it delays the
    eastmost worker's greeting so the west-to-east chain lands in order;
    without it the greeting order breaks.  Frame-model evidence cannot
    see async ordering (the Year 39/40 lesson, again).
  - Y07 size 11: fails live.  Mechanism: the added `if myitem == nothing`
    check shifts the x6 worker's drop by the if's wall-cost, and the
    collation stack demands the original drop order.  Same async class.
  - Y42 size 143: endless loop live -- the mem1->mem2 retarget breaks the
    recovery path in ways the model (which cannot referee this level)
    endorsed.  FINAL.
  - Y68 size 164, Y66 sizes 239 AND 240: fail live.  Even the
    "frame-identical" Y66 fallback diverges -- frame identity in the
    simulator establishes nothing about the game's async execution on
    press levels.  FINAL; the rows stand (172, 254).
  - Y29 size 182: one worker ends holding a cube.  Mechanism: the deleted
    second consecutive `giveto mem3` was a live RETRY -- in the game a
    give at a contended machine can fail outright, and the duplicate
    caught it; the simulator's machine model parks-until-served and never
    fails a give, so it endorsed the deletion.  FINAL, and a prime
    witness for the machine-serve rules still missing from the model.
  - Y25 size 6: "really almost works -- at the end people stand around in
    a queue."  Same signature as Y29: the final gives at a contended
    machine never land in the game while the simulator serves them.
    The incumbent's terminal cleanup exists precisely to handle this.
    FINAL at size 6; the machine-serve read (front/arrival/failure
    rules) is the emulator work that would reopen this class.  This did
    not predict the size-8 unconditional-give form, which retains that
    cleanup and completed live; that separate form was rejected only because
    its 134-second average was slower than the size-9 incumbent's 130.
  - Y44 speed-at-size-8: completes live at displayed speed 2 -- the
    source's claimed 1 did not reproduce, and speed ranks first, so the
    17-command speed-1 row stands.  Re-rolls might show 1; parked until
    someone feels lucky.
- Set/if fold class EXHAUSTED corpus-wide: after Year 36's four live-
  confirmed folds, dead-store analysis of every remaining
  `memN = set X` + `if memN` pair in the published set (five more in
  Year 36, one each in Years 44/51/54, press levels excluded) shows all
  of them READ the stored value later -- none are foldable.  Year 36 is
  locally optimal at 210 for this transform.  Likewise the inversion
  class: the corpus holds six empty-true-arm ifs; five (Year 38) shift
  frames because inverting an or-chain changes short-circuit sampling
  (each sampled term costs time -- a corollary of the async findings),
  leaving the single-comparison Year 58 site as the class's one clean
  candidate; it was subsequently published at size 105 / speed 2.
- Y15 deterministic size 7 (`if myitem == nothing` gate): REFUTED LIVE
  pasted verbatim (2026-08-17) — the run does not complete despite the
  emulator's deterministic 2,167-frame win.  Two prior Y15 size attempts
  died the same way, so Shred Lines live behavior (worker death and/or
  `myitem` semantics on this level) diverges from the model in a way the
  Y21 paste-legality confirmation does NOT cover.  Verdict: no more
  `myitem`-gated Year 15 candidates from simulator evidence alone; the
  size row stays 8.  (The same session's Year 15 SPEED tie-break at 42 —
  a pure step/pickup program with no myitem — completed at 11 and is
  published, so the divergence is specific to the gated forms, not to
  pasting on Year 15.)
- Corpus deletion sweep (2026-08-17, every published solution, singles +
  if/endif pairs + all-pairs on small size rows + greedy chains, 4→25→200
  emulator gates): survivors held back from the queue, with reasons —
  - Y14 speed guard removal (`if s == datacube:`): the result IS the
    published size-4 row, which the game times at 4 versus the guarded
    program's 2 — the emulator prices failed pickups/gives far too cheaply
    (97 versus 130 frames the wrong way round).  Calibration fact, not a
    candidate: failed item actions cost real wall time live.
  - Y48 `takefrom n` deletions (size 4→3 and speed 5→4): the training-goal
    false positive already rejected live; ledgered above.
  - Y13 low-percent `step se` deletion: identical to the queued
    recoverable size-5 entry.
  - Y25 speed: the `giveto mem1` deletion (frame-identical) and broader guard
    removals remain withheld because the machine-serve model is known wrong on
    this level and give deletions were live retries on Y29.  The narrower
    size-8 edit retained the outer availability guard and terminal cleanup and
    completed live, but averaged 134 seconds versus the size-9 control's 130;
    it is rejected above as dominated.
  - Y33 speed 8→7: 1/200 (below the 1-in-100 floor).
  - Y34 speed 33→32 (any of the `nearest` re-reads): 106-175/200 — drops
    a 100% row below the 99% tier without beating the 50+ speed row.
  - Y10 speed `step n`, Y22 speed (all), Y09 `jump b`/`jump c`, Y23
    speed `jump a`/`jump c`: win but with more frames — regressions.
  - Y47 speed 34→33: every always-true `if` pair (the class refuted live —
    those ifs are wall-time pacing) and the eastmost worker's `tell w
    morning` (frame-identical in the model, but the same choreography);
    not queued.
  - Y50 speed 31→30 (steps, a give, listens — several frame-identical or
    faster): a tell/listen choreography level where the model's inbox
    timing is exactly what the live game disputes; not queued.
  Other outcomes from the same sweep: Y22 Solutions50+ speed 7→6 and Y23
  speed 9→8 were published; Y09 speed 15→14 and Y23 low-percent speed
  23→19 (fallback 21) remain queued.  Y54 Solutions50+ speed 37→36 was
  initially queued but later withdrawn under the item-action rule, as recorded
  above.

## Year 60 pickup fusion (size 9→8) — refuted live 2026-08-17

The fused `pickup w,se` (two sequential pickups collapsed into one
direction list) failed in the game: the worker stalled at the **empty**
west square and the run overran the 1,400-second cap.  The emulator's
pickup list scans for the first listed square that has a cube; the live
game evidently does not skip an empty listed square that way.  Year 16's
published record types the same `pickup w,se` and works — there both
squares hold cubes when the command runs, so it never discriminates.
Standing rule: **no candidate may depend on a pickup/give direction list
skipping an empty square.**  The Year 60 size row stays 9.

## Conditional-to-direction-list collapse (whole class) — closed 2026-08-17

Sweep of all 33 sites where an `if`/`else` chooses between two same-verb
direction commands and a single direction-list command would be shorter
(e.g. `if n == datacube: pickup n / else: pickup ne` → `pickup n,ne`).
Emulator survivors: four.  All fall to known live mechanisms —

- Y58 speed 105→102 (`pickup ne`/`pickup nw` merge): depends on the list
  skipping an empty square — the exact mechanism Year 60's live failure
  refuted above.
- Y60 speed 159→157 (three variants of `pickup e` + guarded retry
  `if myitem == nothing: pickup e` merged): deletes a pickup **retry**,
  the class already refuted live on Year 38.

Every other site (step collapses on Y11/12/13/20/30/36/41/42/52/56/62/68,
the Y23 size `step e,w`, Y26's give collapse) loses in the emulator
outright — a step list is a random choice live and in the model, so
collapsing a measured comparison into one throws the information away.
Class closed; do not re-derive.

## Year 56 written-max size 4 — withdrawn before test 2026-08-18

Designed independently (write 99 on any group cube, shred it — the goal
grades the cube by the number it shows going in), then found to be
n05ucc4u's existing LowPercent (both) row, same size 4, same mechanism.
Its LowPercent tier also settles the live behavior: seven workers
converging on the one shredder fail often enough to sit under 50%.
Curation rule: before queueing a "new" candidate, check every tier's
rows INCLUDING (both) files — a (size)-only glob misses them.

## Exhaustive synthesis closures (2026-08-18) — do not re-derive

Every program one command below the record was enumerated and run
(straight-line grammar with the jump-always-last rule; stage 2 adds one
if-block over all conditions; palettes' own commands only, plus
nearest/mem1 forms where available).  Zero winners in every case, so
these size records are optimal within those grammars:

- Year 02 at 3 (324 programs), Year 04 at 3 (341) — no if/jump palettes,
  fully closed.
- Year 03 at 5 (105k) — no if in palette, fully closed.
- Year 07 at 4 (6.5k + 168k stage 2) — fully closed.
- Year 14 at 4 (21k + 377k stage 2) — fully closed.
- Year 28 at 4 (17k with the nearest/mem grammar) — no if, fully closed.
- Year 09 at 5 (122k + 5.35M stage 2) — fully closed.
- Year 11 at 5 (122k + 5.35M stage 2) — fully closed.
- Year 18 at 5 (680k stage 1; stage 2 completed 2026-08-24 with
  19.97M programs, zero winners) — the full size-4 space in both
  grammars is closed; the 5 is optimal within them.
- Year 23 at 6 (3.0M stage 1 at size 5) — stage 2 still open.

Grammar limits for honesty: direction lists, multi-mem programs, and
foreachdir forms are not enumerated; a hypothetical winner would have to
come from those.  Hand-checked separately and dead: Year 35 at 4 is
structurally minimal (acquire/compute/write/replace), Year 17 at 1 is
unbeatable, and the Year 05 low-percent question is already queued as
the absorbing size-2 walker.

## Year 20 guard deletion (38→37) — refuted live 2026-08-19

Live run: infinite loop, timed out at the cap.  The emulator had scored
it 200/200 — but at 304 frames and 42 item actions versus the
incumbent's 314 and 45.  Those two drops were the tell: deleting the
`if sw == datacube:` guard did not run a harmless always-true pickup,
it rearranged who picks which cube.  The emulator explores ONE
deterministic arrival order and that order still finishes; the live
scheduler's order leaves a worker without a cube blocking the line
forever.  The Year 26 else-merge (56→55) was withdrawn untested as the
same class: an item action moved onto a path where it can fail, on a
multi-worker level.

Curation rule from this: **a deletion is only trustworthy when the
model's item-action count is unchanged.**  Fewer frames plus fewer
items = a different choreography that one scheduler order happened to
survive, not an equivalent program.  Applying the rule to the then-standing
queue measured Years 38, 59 and 66 items-identical in fixed-order runs.  Years
38 and 59 still stand, as does Year 9's jump deletion; Year 66 was later
withdrawn after comparative shuffled dispatch fell to 29/100 wins versus the
incumbent's 45/100.  Year 60's 158 showed 319.8 items against the incumbent's
321.3, so it was withdrawn untested as the same class.

## Further synthesis closures (2026-08-19/20) — do not re-derive

- Year 10 at 5: fully closed (5.6k stage 1 + 483k stage 2; its
  step/jump/if palette is completely covered by the two grammars).
- Year 21 at 5: closed at stage 1 (2.03M programs at size 4 over the
  full takefrom/nearest alphabet); stage 2 completed 2026-08-25 with
  45.91M programs, zero winners — the full size-4 space in both
  grammars is closed; the 5 is optimal within them.
- Year 16 at 6: the entire loop-program space at size 5 is exhausted
  (2.46M, jump-last rule), and the halting half followed on 2026-08-22
  (17.21M straight-line programs, zero winners) — the complete size-5
  space in the grammar is closed; the 6 is optimal within it.
- Vocabulary probes (each enumerates every shorter program over the
  record's own commands plus direction/operand variants — evidence,
  not proof): Year 25 at 5 (46.6k), Year 29 at 5 (46.6k), Year 31
  at 6 (4.2k) all survive with zero winners.
- Year 51 at 6: the tight-vocabulary probe at size 5 completed
  2026-08-24 — 7.56M programs over the record's own 23-command
  vocabulary, zero winners.  Same evidence tier as the probes above.
- Year 39 at 6: tight probe at size 5 completed 2026-08-25 — 134,865
  programs over its 9-command vocabulary plus its condition, zero
  winners.  (A first run had inflated the vocabulary with multi-line
  condition fragments and was discarded; the loader now joins split
  conditions before harvesting.)
- Year 68 at 6: tight probe at size 5 completed 2026-08-25 — 290,000
  programs, zero winners.  This finishes the small-row probe program:
  every size row at 6 or below now carries either a full grammar proof
  or a completed vocabulary probe.
- Structural arguments: Year 47 at 3 (the conditional wait is
  irreducible — the leftmost worker must speak unprompted, all others
  must wait; spam-tell forms 0/50), Year 49 at 4 (the trailing step e
  is load-bearing, 0/100 without it), Year 19 at 3 and Year 46 at 2
  (acquire+pass and the forever-loop each need their commands),
  Year 35 at 4 (acquire/compute/write/replace, nothing removable).

## Year 61 at 7 hardening, and the speed-slack survey (2026-08-20)

The README's sole below-world-best row (Year 61 size, 8 against the stat
page's 7) cannot be fixed by tuning the known 7: the distance-writing
walker measures 90% in the model (losses are clock-outs), raising the
propagation bound to 12 reaches only 92.5%, and every other variant
tried (bound 10, cardinal-only walk, west-biased walk) scores 0/200 —
the full 8-direction walk and the <11 bound are both load-bearing.  A
99% seven needs a different algorithm; the vocabulary space at size 7
is too large to enumerate.

A frames-versus-displayed survey of every speed row found the apparent
slack (Years 17, 19, 47, 49, 50, 56) is all cadence-bound — greeting
chains, contended machines, forever-goal observation windows — where
frame cuts cannot move the displayed clock.  The one borderline is
Year 15 speed: 622 frames is 9.95 s of frame time displaying 11, so a
~60-frame walk redesign could plausibly show 10; no deletion exists
(swept), it would need new choreography.

## The never-searched small rows, the trivial-program sweep, and two emulator blind spots (2026-08-21)

First one-command-shorter vocabulary probes over every size row no
closure, probe, or queue entry had ever touched (gate cap at 3x the
incumbent's measured frames — an earlier 30,000-frame ceiling blinded
the slow levels, so Years 30 and 58 were re-run before counting):

- Year 59 at 3 (34 programs), Year 58 at 3 (1,610), Year 30 at 3 (45),
  Year 32 at 4 (5,632), Year 50 at 4 (320,892): zero winners each.
- Year 24 at 4: of 144 programs, exactly 14 win in the emulator, and
  every one strips the incumbent's machine-safety guards (unguarded
  `giveto s,e` loops, or one-guard halves).  All rest on a give-list
  falling through the printer square — the live-refuted printer-return
  family — so none are queueable, and no machine-safe size 4 exists in
  that vocabulary.  The 5 stands.
- Year 48 at 3: of 10,143 programs the only four winners are the
  `takefrom s` + nearest-shredder family — the live-rejected
  direct-south choreography riding the instructor-shred false
  positive.  Nothing else at size 3.
- Year 44: void, see below.

The all-levels size-1/2 exhaustive sweep (straight pairs, label-jump
loops, one-line if-blocks, one-body foreachdir assignments, and the
`a: jump a` idle loop; 8-trial gate at the same 3x cap, 200-trial
confirm; any >=1 percent win below a level's best would have been a
LowPercent record row): every level from the size-15 record down
through the size-5 tier — zero hits.  No trivial low-percent cheese
hides under any large record.  Stated limit: the 8-trial gate
under-samples sub-10-percent rates; it is a net for big cheese only.

Two emulator blind spots mapped while measuring incumbents (also in
the private notes): Year 44 is unscorable as a WHOLE LEVEL (its
low-percent 4, the 99+ size 5 and the 99+ speed 17 all score 0/50 at
full cap — the fashion mechanic is not modelled; every emulator result
on Year 44 is void, including this sweep's).  Year 34 is blind on its
SIZE ROW ONLY: the level models fine (speed row 50/50), but the size-7
record leans on comparison semantics of a nearest-result register
(`c < mem1`, `mem1 == something`) the emulator gets wrong.  Never
seed or screen candidates from either until those are read out live.

Also closed: a frame-minimizing mutation search over the Year 15 speed
row (96,000 mutants at size <=42, full-win gate) found nothing below
622 frames — the row is locally tight under mutation reach, matching
the survey's "needs new choreography" verdict.

## Goal-audit round two: spawn luck, held-cube exemptions (2026-08-23)

- Spawn-luck audit: a single-step no-op run 2,000 trials on every one of
  the 64 levels — zero wins anywhere.  No goal in the corpus is ever
  satisfied by its random spawn at a 0.2-percent-or-better rate, so no
  free size-1 LowPercent row exists.  (The earlier size-1/2 sweep's
  8-trial gate could not see below ~10 percent; this closes that gap.)
- Year 57 held-cube reduction: the neighbor-count goal grades floor
  cubes only (the level's own tip documents the mechanic) and every
  cube starts at value 0, so the goal is equivalently "no two floor
  cubes adjacent" with no writing at all.  Dead on arithmetic: 52
  clustered cubes against 10 hands needs a 42-cube independent set
  that the clusters cannot contain, and the palette has no `nearest`
  for ferrying the surplus into the map's holes.  Do not revisit
  without a new mechanism.
- Year 55: the flower goal requires at least one INTACT ring to grade
  (breaking every flower is a loss, not a vacuous win); solving one
  flower costs the same loop the record already runs.  No exploit.
- The remaining end-state goals (sorted grid, both defrags,
  mode-counts, identify-line, decrypt-left-exit) were re-read with the
  counter-exploit lens: each genuinely demands the work its record
  performs.  One corroboration fell out: the Year 41 goal requires all
  workers gone and its community record culls them by divide-by-zero —
  the same kill mechanism the queued Year 44 static-cull lead relies
  on, so that mechanism is established rather than speculative.
