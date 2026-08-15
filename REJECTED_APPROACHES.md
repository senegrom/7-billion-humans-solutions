# Rejected and Superseded Approaches

These experiments are kept so failed ideas are not rediscovered and mistaken
for records.  Candidates that still need live-game verification remain in
[SOLUTIONS_TO_TRY.md](SOLUTIONS_TO_TRY.md).

- Y24 size 3: emulator false positive; real handoff semantics do not sustain it.
- Y15 public size 6: 96/100 capped emulator wins, but superseded in both size
  and reliability evidence by the retained novel size-5 candidate.
- Y15 size-5 give-first ordering: 300/300, but average and worst-case timing
  were slightly worse than the retained pickup-first ordering on the same
  seeds (191.4 vs 190.9 average; 387 vs 378 worst).
- Y15 symmetric size-5 gate `(shredder and holding) or north-cube`: 300/300,
  but much slower than the retained empty-worker gate (265.5 average, 727
  worst).  This exhausts the only other non-dominated one-IF predicate family.
- Y16 size-4 random `step s,e` route: a finite three-cube path exists, but two
  buffered witness batches timed out without yielding a result and the static
  route probability is too low for a practical live test.  Retain as theory,
  not as a queue entry.
- Y10 random-walk size 2: 0/100 capped emulator wins; any winning path is too
  rare to justify live-game queue space without stronger evidence.
- Y10 speed size-32 branch fusion: it still wins, but takes 867 model frames
  versus the incumbent's 790 and is therefore not a speed-record tie-break.
- Y07 deterministic size-9 pickup linearization: it wins, but adds a 93-frame
  invalid-pickup bubble and slows modelled speed from 4 to 5.  The retained
  error-free size-11 candidate is the correct secondary-size lead.
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
- Y25 external persistent-loop reduction: the source program is 32 commands at
  129 seconds and the proposed unconditional-tail form is about 30 commands at
  the same speed.  Both are dominated by the local size-9/live-129 endpoint and
  the retained size-8 candidate.
- Y25 size-8 unconditional-give candidate: it wins and is one modelled second
  faster than the incumbent, but is superseded by the retained guarded-loop
  size-6 candidate, which is two commands smaller (three below the incumbent)
  and 22,919 frames faster.
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
- Y39 unguarded-printer size 5: 0/100 at a 20,000-frame diagnostic cap; random
  requeueing did not produce the required exactly-five-sheets-per-worker state.
- Y39 end-only speed size 172: superseded by the retained size-167 composition,
  which includes the same terminal-`end` deletion plus five neutral diagonals
  and remains exactly 2,581 frames.
- Y40 speed branch-`b` and branch-`c` terminal-`end` deletions: each failed
  0/1 at the real cutoff, while the incumbent wins in 2,777 frames.  Both
  workers fall through into later branches before the global goal is complete,
  so neither `end` is post-goal dead code.
- Y40 size-170 PR/diagonal and size-175 PR/dead-calc candidates: both are
  superseded by the retained size-169 composition, which combines every safe
  edit and remains exactly 2,697 frames/100 item actions like size 175.
- Y41 random-exit size 6: a finite all-west-while-carrying then north-to-hole
  route exists, but even the relaxed geometry estimate is only about 1e-88;
  there is no practical witness search or live test.
- Y42 size-8 loop fusions: both the outer-delivery and unified random-walk
  variants failed 0/5; the separate search and delivery drifts are material.
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
- Y58 size-3 random, nearest, cardinal-pick, diagonal-pick, and one-shot
  pruning families produced no witness across their bounded screens.  Exact
  graph optimization shows a win must remove at least 12 interior cubes and
  at most 8 perimeter cubes; the tested policies have the wrong removal bias.
- Y60 alternate size 9: superseded by dmr's public 0/200-failure size-9 record.
- Y63 size 9 rewrite: 0/10.
- Y64 size 7 rewrite: 0/5.
- Y64 speed size-99 deletion of one initial `mem1 = set myitem` cadence pad:
  0/1 at the real cutoff while the untouched size-100 incumbent won in 561
  frames.  Even one apparently dead store is phase-critical on this level.
- Y65 size 13 rewrite: 0/52.
- Y66 further padding cuts: deleting one store from the later 14-store block
  fails; deleting the earlier singleton after its first shuffle wins but slows
  from 1,552 to 1,574 frames/modelled speed 26.  Size 240 is the exact-timing
  fallback and size 239 in the live-game queue is the only retained aggressive
  boundary.
- Y68 unconditional size 5: 0/20; removing the side-hole guard did not produce
  an observable low-percent win.
- Y68 tell-only speed sizes 171/170: superseded as primary candidates by the
  size-164 bypassed-wrapper core, which removes eight charged but statically
  unreachable wrapper commands without changing executed cadence.  The tell
  cuts remain optional size-163/162 follow-ups only after the core works live.
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
  than the size-41 base, but superseded by the retained position-preserving size-39
  fusion, which removes both walks and wins 100/100 with a better full timing
  range.
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
  Y38 (140) and Y59 (141) diagonal stacks withdrawn untested as the same
  class.  Y40 falls back to the PR-92 ladder (176 live-verified 36, then
  175).  Frame-based speed evidence anywhere in the queue is downgraded
  to "requires live incumbent control first".
