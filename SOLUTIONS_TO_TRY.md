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
40) showed that frame-identical simulator evidence does NOT establish the
displayed speed: diagonal step substitutions regressed 36 to 41 in both
tests.  (The cause is not wall time, as first thought: the game counts a
fixed 16 ms per tick, averaged over the 25 random rounds it plays after a
visible win, so these were timings the model got wrong.)  Every speed
tie-break below therefore requires a live incumbent control run first;
discard the candidate on any displayed-speed regression.  Win/loss and
size evidence is unaffected.  (Re-run on 2026-10-23, the corrected model
reads all three refuted candidates slower than their incumbents, the
Year 40 ones by about the live margins, so a candidate it reads slower
is now dropped without a live run; one it reads tied or faster still
needs the control.)  (Re-run on 2026-10-26, with the printer's serve
timed as the game's, the model reads the Year 40 incumbent and its two
refuted candidates at exactly their live 36, 38 and 41 s, but the Year
39 stack as a tie with its incumbent, 35 against 35, where the game
showed 41 against 36: a modelled tie still needs the control.)  (Re-run
on 2026-11-06, with the walks timed as the game's, the model still reads
the Year 40 three at 36, 38 and 41 s, and now the Year 39 stack slower
than its incumbent, 36 against 35, as the game showed.)

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
(Decoded exactly on 2026-10-19; see CLAUDE.md: a list holds five
things, and only some lists survive one wall, hole or cube past the
fifth.  The corrected model crashes the Year 30 take in every world.)

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

**Two speed ladders parked (2026-10-20):** a second review of the
emulator against the game's own code shows that the Year 67 ladder's
deleted tells are the presser's clock rather than padding, and that the
Year 68 ladder can never get the passing control run it needs.  Both now
sit with the parked long shots at the end, and neither needs game time.

**Completions count only once kept (2026-10-21):** after a visible win
the game plays 25 fresh random rounds and keeps the completion only if at
least 14 of them win; otherwise it says the solution failed more than 50
percent of the random tests and saves nothing.  This holds on every level
from Year 09 on, and on the earlier ones once Year 09 is beaten.
CLAUDE.md's floor is therefore now a 35% win rate per run (about one kept
completion per 100 attempts), and the low-percent leads, all well below
it, now sit with the parked long shots.  After any failed round the game
replays that room until you leave the level, so go back to the elevator
before counting another attempt.

**Model re-measured again (2026-10-23, after the second review's fixes
landed):** every queued program was measured again; the entries quote
the new figures where they moved.  The Year 62 ladder is withdrawn: both
rungs now win 779 runs in 1000 against the incumbent's 980.  The Year 23
low-percent speed tie-break comes back from the parked long shots: its
size-21 rung wins 43% of runs, a kept completion about once in 17
attempts.  Yifeixuer's Year 40 program now runs in the model, which
reads it about 2 s slower than the incumbent, and the Year 15 community
six reads 67%, near its public 64%.  A candidate of our own that the
model reads slower than its incumbent is now dropped without a live run
(CLAUDE.md); none of the remaining tie-breaks does.

**Model re-measured for the game's hand-off (2026-10-26):** the model
now hands a cube from one worker to another as the game does (the giver
or taker meets its partner, then a throw, the cube's flight and a
catch), hands a printer's sheet over a tick sooner, grades the shredding
rooms on the cubes left in them, and keeps a worker that reaches into a
shredder on its square until the shredder has it.  Every queued program
was measured again, and four entries move.  The Year 15 community six
now wins 76% of plain runs and the Year 30 probabilistic four 76% on
both screens; yifeixuer's Year 40 program reads 3 s slower than the
incumbent, which the model now reads at its live 36; the Year 30
alternate five still reads about 8 s faster than the size-5 row plain,
but slower shuffled.  A give to a worker, or a take from one, meets its
partner on a tick that depends on the order the two take their turns,
which the shuffled screen redraws every frame and the game never does,
so a program that hands cubes between workers can lose far more runs
shuffled than plain: the Year 15 six wins about 45% there (see
CLAUDE.md's jitter rule).

**Model re-measured for the trade (2026-10-28):** a walker in the model
now trades places with a worker standing idle at its pin's stop, as in
the game, and pushes a worker aside from the tick its program reaches
its end.  Every queued program was measured again, and two entries move,
by little: the Year 30 probabilistic four wins 154/200 plain and 151/200
shuffled (301/400 on both screens), and yifeixuer's Year 40 program runs
a frame faster, still 3 s slower than the incumbent by the game's rule.
The Year 15 community six keeps its figures.

**Model re-measured for the approach square (2026-10-31):** a giver or
taker walking up to its partner in the model now heads, stride by
stride, for the floor square beside the partner nearest it, whoever
stands there, as the game does.  It waits for a worker it cannot push
aside, where it used to walk round to another square, and a worker
waiting there on the same partner holds all three until a fourth
worker's walk trades squares with the walker.  Every queued program was
measured again, and two entries move: the Year 30 probabilistic four
wins 131/200 plain and 130/200 shuffled (250/400 and 243/400 over 400
runs), and the Year 15 community six 142/200 plain and 93/200 shuffled
(730/1000 and 185/400).  On the game's own code, on the real levels, the
walk takes the new square wherever the old and new walks part, 20 times
in 20, and stands still in the same shapes.  Both programs stay above
the floor and keep their places in the list.

**Model re-measured for the look (2026-11-01):** a take aimed by direction
that finds a printer in the model now looks at its squares first and
walks to the printer's front only once the look is over, as the game
does; it used to set off at once and pay what was left of the look on
arrival.  Every queued program was measured again, and one entry moves:
the Year 30 probabilistic four wins 119/200 plain and 125/200 shuffled
(238/400 and 243/400 over 400 runs).  The fall over 200 runs is the
seeds, not the change: over 2,000 runs it wins 1,262 plain, against
1,273 before.  On the game's own code, on the real levels, each of the
222 printer takes by direction in its runs of eight published programs
ends its look 33 ticks after it came up, before the worker moves.  The
four stays above the floor and keeps its place in the list.

**Model re-measured for the list draw (2026-11-02):** a step naming several
directions in the model now picks its square at random as the game
does, passing over a square with a body or a claim on it, and gives way
to a walker it blocks while it waits, keeping the square it picked; a
step at a remembered thing gives way too once it has set out; and the
model draws the game's random numbers for a look aimed by several
directions, and twice at a printer's serve, as the game does.  Every
queued program was measured again, and four entries above the parked
long shots move.  The Year 15 community six wins 146/200 plain and
77/200 shuffled (698/1000 and 165/400), 68.5% of 8,000 plain runs
against 74.3% before; the Year 30 probabilistic four 115/200 and
123/200 (240/400 and 215/400; 1,198/2,000 plain against 1,262);
yifeixuer's Year 40 program now reads level with the incumbent, 36 s by
the game's rule; and the Year 30 alternate five reads slower than the
size-5 row over 200 worlds on both screens but faster over 1,000.  On
the game's own code, on the real levels, the model now follows all four
runs of the Year 15 six tick for tick, every square it draws included,
and the three whole runs of the Year 30 four for 1,617 to 9,527 ticks.
No entry falls below the floor, and the order stands.  Four parked long
shots move a little.

**Model re-measured for the bodies' paths (2026-11-03):** a worker's
body in the model now takes the game's path through a stride.  A
direction step that swings round a floor cube curves out through the
game's points, and one rising diagonally onto a cube climbs through the
square straight above the one it leaves; a walk along a route steers
each stride's body through its points and leaves it short of a square's
centre where a stride ends in the middle of the route, walking it on to
the centre while the next stride is refused; and a `foreachdir` look
finds whoever stood on its square as the game's tick began.  Every
queued program was measured again, and six entries above the parked long
shots move, by little.  The Year 15 community six wins 147/200 plain and
76/200 shuffled (691/1000 and 162/400), 68.8% of 8,000 plain runs
against 68.5%; the Year 30 probabilistic four 125/200 and 106/200
(249/400 and 208/400), 60.1% of 8,000 plain runs as before but 2,065 of
4,000 shuffled against 2,194, and about 1 run in 70 now crashes (see its
warning); the Year 38 speed tie-break wins 989/1000, and its fallback
and the incumbent 997/1000, from 992 and 998; the Year 40 incumbent now
runs a frame faster than yifeixuer's program, both still 36 s by the
game's rule; the Year 38 community six-seven averages 438 frames, from
437; and the Year 30 alternate five reads 4 s faster than the size-5 row
over 1,000 plain worlds and level with it shuffled.  Every rate and
average over 200 to 1,000 runs moves within noise.  On the game's own
code, on the real levels, the model still follows all four runs of the
Year 15 six tick for tick, and the three whole runs of the Year 30 four
now for 1,617 to 19,923 ticks.  No entry falls below the floor, and the
order stands.  One parked long shot moves a little.

**Model re-measured for the walks' timing (2026-11-06):** the model now
times a worker's walks as the game does.  A walk along a route that ends
short of its last square's centre leaves the next walk a tick late; a
step at a remembered worker that ends beside it once its stride is
refused, and an errand given up where it stands, forget the refused
stride, so the worker stands and walkers go round it; every item command
that walks to its thing sets out a tick after it comes up, and a feed
hands its cube to the shredder a tick or two sooner; a worker that lands
from a direction step, or from a step at a remembered thing on its
walk's last square, stands on its square to the other workers' routes
from the tick it lands, unless another direction step comes next; and a
walk refused while its body stands short of its square's centre walks
the body on to the centre first, claiming its next square only once the
body is there.  Every queued program was measured again, and six entries
above the parked long shots move.  The Year 15 community six wins
146/200 plain and 75/200 shuffled (691/1000 and 160/400), 68.8% of 8,000
plain runs as before; the Year 30 probabilistic four 120/200 and 106/200
(228/400 and 214/400), 58.4% of 8,000 plain runs against 60.1%, and
about 1 run in 53 now crashes, against 1 in 70 (see its warning); the
Year 38 speed tie-break wins 988/1000 plain and 989 shuffled, and its
fallback and the incumbent 997/1000 as before; yifeixuer's Year 40
program runs 2,298 frames and wins 223/1000 shuffled, from 263, while
the incumbent runs 2,295, as on the game's own code, both still 36 s by
the game's rule; the Year 38 community six-seven averages 437 frames,
from 438; and the Year 30 alternate five now reads faster than the
size-5 row on both screens over 200 and over 1,000 worlds, within noise.
Every other rate and average over 200 to 1,000 runs moves within noise.
The model now reads the Year 39 diagonal stack slower than its
incumbent, as the game showed (see the speed note above).  On the game's
own code, on the real levels, the model now follows all three whole runs
of the Year 30 four and the Year 40 incumbent's run to their ends, and
matches every worker's hands in the four runs of the Year 15 six too.
No entry falls below the floor, and the order stands.  Two parked long
shots move a little.

**Model re-measured for the first stride and the bodies' reads
(2026-11-09):** the model now follows six more of the game's rules.  A
shredder counts a cube as its blades take it in, 20 ticks after the
hand-over, and a room graded on how many cubes its shredders took is won
then; a walker asks for its next square only with its first stride, so a
walk taken up and not yet set out trades places with nobody and gives
way to nobody, and chooses its first square as it sets out; a walker's
fall into a hole begins as its body nears the hole's middle, two ticks
before a straight stride into it ends, and frees its squares then; a
step at a remembered worker looks for that worker where its body is;
`nearest worker` measures every worker's body where it is, as the bodies
stand at the reader's turn in the tick it takes the command up; and a
calc that divides by zero kills its worker 79 ticks after it is taken
up, the worker staying in the room while it dies.  Every queued program
was measured again, and seven entries above the parked long shots move.
The Year 38 speed tie-break wins 994/1000 plain and 995 shuffled at 617
frames, and its fallback and the incumbent 997/1000 at 627 to 628, still
9 s against 10; the Year 59 tie-break runs 444 frames against its
incumbent's 429, 7 s against 6 by the game's rule, the incumbent at its
live 6, so the model now reads it a second slower, and it leaves the
queue (CLAUDE.md's speed rule; see REJECTED_APPROACHES.md); the Year 58
list merge and its incumbent run 187 frames, 2 s by the game's rule, the
record's live speed, and both win 844/1000 shuffled, the same runs, from
1000; yifeixuer's Year 40 program runs 2,311 frames, where its last cube
lands on the game's own code, 16 frames behind the incumbent, both still
36 s by the game's rule; the Year 38 community six-seven wins 670/1000
at 439 frames; the Year 30 probabilistic four 117/200 plain and 106/200
shuffled (228/400 and 219/400), 58.3% of 8,000 plain runs against 58.4%,
and still about 1 run in 53 crashes (see its warning); and the Year 30
alternate five still reads faster than the size-5 row on both screens
over 200 and over 1,000 worlds, now by 1 to 12 seconds, within noise.
The Year 15 community six keeps its figures.  On the game's own code, on
the real levels, the model still follows all three whole runs of the
Year 30 four, the Year 40 incumbent's run and the four runs of the Year
15 six to their ends, and now the Year 59 speed record's run too.  No
entry falls below the floor, and the order of the rest stands.  Three
parked long shots move: the Year 44 four and three a few runs in 1,000,
and the Year 68 ladder's incumbent, which the model now wins every plain
run at its live 16 s.

**Model re-measured for the trades, the walk-ups and the glances
(2026-11-13):** the model now follows thirteen more of the game's rules.
A walker with a route that a swap, a give-way or a rotation moves
strides as on a walk of its own, from where its body stands, and lands
as after a stride of its own, settling a tick before it acts again and
still wanting the square it was bound for; the walker whose step closes
a swap or a rotation sets off on that tick, whatever it was doing, and
the others on their next turns; a printer serves every sheet it is asked
for, in a room graded on what its shredders took too; a step at a
remembered thing is over on the tick its walk lands where it finds its
thing; a step that gives way round a floor cube, its own square still to
walk on to, lands at the last point before the centre and walks on from
there; a direction stride's points are laid from the square the worker
held as it set out; a give's or a take's walk up to its partner heads
for the floor square beside the partner nearest the walker's body, asks
for its first square only as it sets out, and stands to the other
workers' routes from the tick it lands; a `foreachdir` glance at a
square where a worker stands with no cube names the worker, and a glance
reads its square's floor a tick sooner, so it still finds a cube that a
worker earlier in the turn lifts as the glance is taken up; and a memory
slot reads a cube's number wherever the cube is, in a falling worker's
hands or in the air between two workers.  Every queued program was
measured again, and two entries above the parked long shots move, both
on Year 30 and both within noise.  The probabilistic four wins 111/200
plain and 120/200 shuffled (223/400 and 232/400), 58.4% of 8,000 plain
runs against 58.3%, and about 1 run in 58 crashes, against 1 in 53 (see
its warning); and the alternate five still reads faster than the size-5
row on both screens over 200 and over 1,000 worlds, now by 2 to 15
seconds.  Every other entry keeps its figures as quoted: the Year 15
nearest loop five runs 994 frames, from 992, still about 15 s, and the
Year 38 speed tie-break loses one run more in 2,000 on each screen,
neither in its first 1,000.  On the game's own code, on the real levels,
the model still follows all three whole runs of the Year 30 four, the
Year 40 incumbent's run and the four runs of the Year 15 six to their
ends, and now the Year 30 speed record's run, the Year 59 size record's
and the Year 50 size record's too.  No entry falls below the floor, and
the order stands.  No parked long shot moves.

**Next session (CLAUDE.md's floor: 35% of plain runs):**

1. **Year 23 paste test, size 2**: one paste and one look at the editor,
   about a minute.  It decides whether the game keeps a command the
   level's editor does not offer; if the run then completes, it is a
   size-2 record against 6.  Expect a no: the one hidden feature
   pasted before (`myitem` on Year 15) did not work.
2. **Only if item 1 kept its command:** the Year 16 four (record 6) and
   the Year 15 five (records 8 and 6), each a couple of short runs.
3. **Speed tie-breaks**, each with the incumbent control run first,
   every run under a minute: Year 58 at 97, Year 38 at 140, Year 21 at
   40, Year 40 at 120 (then 123), Year 29 at 199, then Year 40 at 93
   (yifeixuer's 2018 program, which the model now reads level with the
   incumbent, 36 s by the game's rule).
4. Optional control: the Year 15 community six, already the Solutions50+
   row on public evidence (16/25 live); about 68% here.  Skip it if the
   Year 15 five in item 2 completes.
5. Optional, for a patient session: confirmations of the published Year 38
   community speed 6-7 and Year 30 probabilistic four (read its crash
   warning first); and the Year 30 alternate five, an unpublished edit of
   the size-5 row whose only claim is a small model speed edge that a live
   A/B has to confirm.
6. Optional, the one low-percent item above the floor: the Year 23
   low-percent speed row at size 21, then 19 (in the model a completion
   is kept about once in 17 attempts and once in 95); stop at the first
   displayed-speed regression.

The other low-percent leads sit with the parked long shots at the end:
under the game's 25-round check none of them can realistically be kept.

**Rows taken from another table (2026-11-22):** twenty-two rows of the
record table now carry programs from the abfipes12 table, whose times
are read from the game to the tick, and three entries leave this queue
with them.  The Year 09 tie-break at size 14 is settled: that table
lists this very program, its labels renamed, at 3.68 s, so it is the
Year 09 speed row now.  The Year 58 list merge at 102 and the parked
Year 68 ladder at 170 and 171 are overtaken by rows of 99 and 153
commands (see REJECTED_APPROACHES.md).  Yifeixuer's Year 40 program now
stands against a 126-command row.

**Search resumed (2026-11-22):** a scan for speed rows at fewer commands
runs again.  It trims each published speed program, one or two commands
at a time, and keeps what the model runs in the same shown second,
world for world, with the same item count and no worse shuffled.  Its
first entry is the Year 21 tie-break at size 40 below; the Year 29
tie-break at size 199 and the Year 58 and Year 40 ones followed on
2026-11-23.  Where a level's runs do not vary, a candidate is also run
on the game's own code; the entries say where that has been done.

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
- Left out on purpose: the same trick on Budget Brigade 2 at size 4 (the
  all-left relay plus `write 0`).  Its give list names the printer, and
  a give list takes a printer before any worker, which is how the relay
  six died in the game; the model, which follows that rule since
  2026-10-18, loses it every run (0/200 plain and shuffled, 2026-10-26).
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
  1000/1000 shuffled**, about 7 s of game time (re-measured 2026-10-23).
  The three workers always pick three different cubes.
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
  1000/1000 shuffled**, about 15 s of game time (re-measured 2026-10-23;
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

### [ ] Year 58 - Good Neighbors - speed tie-break at size 97 (fallback 98)

- **Paste-ready program:** [SolutionsToTry/Year 58 - Good Neighbors - speed tie-break at size 97.txt](<SolutionsToTry/Year 58 - Good Neighbors - speed tie-break at size 97.txt>)
- **Fallback (size 98):** [SolutionsToTry/Year 58 - Good Neighbors - speed tie-break fallback at size 98.txt](<SolutionsToTry/Year 58 - Good Neighbors - speed tie-break fallback at size 98.txt>)
- Goal: retain the displayed speed record of 2 while reducing the size
  from 99 to **97** (commonnickname's program).
- Exact edit: delete the test `if mem4 != worker:` with its `endif`,
  and the test `if mem4 == worker:` with its `endif`; the commands each
  guarded stay, a tab to the left.  The fallback deletes only the first
  pair.
- What changes: five workers reach the two tests.  For three the test
  holds, and they run a tick sooner.  The other two are surplus workers
  that the record sends straight to its closing division; without the
  tests they run the guarded commands instead (one walks at the thing
  it remembers, the other takes two steps and tries a pickup that is
  refused), and both are still alive, with empty hands, when the room
  completes.
- Evidence (2026-11-23): on the game's own code the 97-command program
  completes after 185 ticks (2.96 s), a tick sooner than the record's
  186, and stays complete.  The model agrees to the tick: 1000/1000
  plain and 1000/1000 shuffled at 185 frames, the record 186 on both
  screens, the fallback 185.  Its count of non-step effects differs
  from the record's, 36 against 38: the surplus workers' two divisions
  and one glance are never reached, and one pickup is added.
- Suggested live test: incumbent once as control, then the candidate;
  2-second runs; discard it on any displayed-speed regression.
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
  0.1 of each other.  Re-measured 2026-10-23 with the second review's
  fixes: the candidate 992 on both screens at 617 frames, the fallback
  and the incumbent 998 on both at 627; by the game's rule the model
  reads the candidate at 9 s and the other two at 10 (live 9-10), so the
  live control decides.  Re-measured 2026-11-03, with the bodies
  following the game's paths: the candidate 989 on both screens at 616
  frames plain and 617 shuffled, the fallback and the incumbent 997 on
  both at 628, a few runs fewer each, within noise; still 9 s against 10
  by the game's rule.  Re-measured 2026-11-06, with the walks timed as
  the game's: the candidate 988 plain and 989 shuffled at 615 frames,
  the fallback and the incumbent 997 on both screens at 627, within
  noise; still 9 s against 10 by the game's rule.  Re-measured
  2026-11-09, with the first stride and the bodies' reads as the game's:
  the candidate 994 plain and 995 shuffled at 617 frames, the fallback
  and the incumbent 997 on both screens at 627 to 628, within noise;
  still 9 s against 10 by the game's rule.
- Suggested live test: incumbent once as control, then the candidate;
  9-second runs.
- Note (2026-11-23): an open pull request to the abfipes12 table
  (motlin's, of 2026-11-22) has this row at 135 commands, which the
  model wins 200/200 on both screens at 9.9 s on average.  If that
  program is taken into the table, this entry is overtaken.
- Result: _not yet tested in the game_.

### [ ] Year 21 - Big Data - speed tie-break at size 40

- **Paste-ready program:** [SolutionsToTry/Year 21 - Big Data - speed tie-break at size 40.txt](<SolutionsToTry/Year 21 - Big Data - speed tie-break at size 40.txt>)
- Goal: retain the displayed 16-22 while reducing the size from 41 to
  **40** (commonnickname and abfipes12's program).
- Exact edit: delete the first `if myitem < 50:` and its `endif`, right
  after the opening `step s`, `takefrom s` and `if myitem == datacube:`;
  the four commands it guarded (`step ne`, `drop`, `step s`, `jump a`)
  stay.
- Why the test is never needed: that first take gives a sheet only to
  the worker standing north of its printer, and a fresh printer's first
  sheet is always a 0 (a sheet carries the number rolled at the serve
  before it), so the test holds every time it is reached.
- Emulator evidence (2026-11-22): 1000/1000 plain and 1000/1000
  shuffled, as the incumbent.  On the plain screen every world ends
  exactly one frame sooner than the incumbent's, the deleted test's own
  frame (1,354 frames on average against 1,355, 21.66 s against 21.68),
  with the same item count (63.1).
- Suggested live test: incumbent once as control, then the candidate;
  20-second runs.  A kept completion at 40 commands inside the
  incumbent's range takes the row.
- Result: _not yet tested in the game_.

### [ ] Year 40 - Printing Etiquette 2 - speed tie-break at size 120 (fallback 123)

- **Paste-ready program:** [SolutionsToTry/Year 40 - Printing Etiquette 2 - speed tie-break at size 120.txt](<SolutionsToTry/Year 40 - Printing Etiquette 2 - speed tie-break at size 120.txt>)
- **Fallback (size 123):** [SolutionsToTry/Year 40 - Printing Etiquette 2 - speed tie-break fallback at size 123.txt](<SolutionsToTry/Year 40 - Printing Etiquette 2 - speed tie-break fallback at size 123.txt>)
- Goal: retain the displayed 36 while reducing the size from 126 to
  **120** (commonnickname and senegrom's program).
- Exact edit: six steps go, each of which only places a worker before
  it takes a sheet or sets one down: the `step ne` and `step e` between
  the first `drop` and the `takefrom mem1` after it; the `step ne`
  between `drop` and `takefrom mem1` under `if c == nothing:`; the
  `step nw` before `step n` and the label `a:`; the first of the three
  `step n` before `write 3`; and the `step e` between `drop` and
  `takefrom mem1` just above the label `c:`.  The fallback deletes only
  the first three.
- Why they can go: the level lets a sheet lie anywhere on the floor,
  and a `takefrom mem1` walks its worker to the printer by itself.
- Evidence (2026-11-23): the model runs the incumbent in 2,292 frames,
  36.672 s, the time the table it came from gives.  The 120-command
  program wins 1000/1000 plain at 2,309 frames (36.94 s) and 1000/1000
  shuffled, with the same item count (91): 3.5 frames short of 37 s.
  The fallback wins 1000/1000 on both screens at 2,296 frames plain
  (36.74 s), and on the game's own code it completes after 2,296
  ticks, the same count.  The six steps are not independent: the
  workers queue at one printer, and other subsets of them run seconds
  slower or lose a sheet (without that `step n` alone every run is
  lost), so paste the files as they are.
- Suggested live test: incumbent once as control, then the 120; if it
  shows 37, the 123; 36-second runs.
- Result: _not yet tested in the game_.

### [ ] Year 29 - Biometric Access - speed tie-break at size 199

- **Paste-ready program:** [SolutionsToTry/Year 29 - Biometric Access - speed tie-break at size 199.txt](<SolutionsToTry/Year 29 - Biometric Access - speed tie-break at size 199.txt>)
- Goal: retain the displayed 52 while reducing the size from 200 to
  **199** (motlin's program).
- Exact edit: delete the `step e` that stands right before `jump g`.
- Why it can go: only one worker reaches that step, and it stands in
  the two-square passage with a wall to its east, so the step is
  refused and costs it one tick; it jumps on from the same square
  either way.
- Emulator evidence (2026-11-23): 1000/1000 plain and 1000/1000
  shuffled, as the incumbent.  On the plain screen every world runs
  3,278 frames against the incumbent's 3,277 (52.45 s against 52.43):
  that worker is a tick ahead, the others meet it differently in the
  passage, and the last cube is shredded one frame later.  The item
  count is the same (140), and the next shown second is 34 frames
  away.  Shuffled, the two average 55.4 s and 55.1 s.
- Suggested live test: incumbent once as control, then the candidate;
  52-second runs.
- Result: _not yet tested in the game_.

### [ ] Year 40 - Printing Etiquette 2 - yifeixuer's speed program at size 93

- **Paste-ready program:** [SolutionsToTry/Year 40 - Printing Etiquette 2 - yifeixuer speed at size 93.txt](<SolutionsToTry/Year 40 - Printing Etiquette 2 - yifeixuer speed at size 93.txt>)
- Goal: match the speed record of 36 at size **93**, against the
  incumbent's 126 (176 until 2026-11-22).
- Source: yifeixuer's Steam post "Year 40 36s" of 2018-10-06
  ([thread](https://steamcommunity.com/app/792100/discussions/0/1728711392737304334/)),
  from game version 2145.  The same day commonnickname withdrew the
  176-command edit (upstream pull request #92) in its favour: "Solution
  by yifeixuer looks much better and he got it first."  It never reached
  either repository.
- Emulator evidence: until 2026-10-18 the model lost it every run.  With
  the second review's fixes (2026-10-23) it wins 1000/1000 plain at 2,478
  frames, 39 s by the game's rule, but only 262/1000 shuffled, while the
  incumbent wins 1000/1000 on both screens at 2,322 frames (37 s; live
  36).  So the model reads it about 2 s slower and far more fragile, and
  under the speed rule a candidate of our own would be dropped.  With the
  printer's serve timed as the game's (2026-10-26) it runs 2,457 frames,
  still 39 s, and wins 263/1000 shuffled, while the incumbent runs 2,302
  frames, 36 s, its live speed, and wins every run on both screens: 3 s
  slower by the game's rule.  With walkers trading places with a worker
  idle at its pin's stop (2026-10-28) it runs 2,456 frames, still 39 s,
  and still wins 263/1000 shuffled.  With steps at a remembered thing
  giving way once they have set out, as the game's do (2026-11-02), it
  runs 2,292 frames, 36 s by the game's rule, against the incumbent's
  2,302 frames, also 36 s, and still wins 263/1000 shuffled; on the
  game's own code its last cube lands at about frame 2,311 (36.98 s).
  With the bodies following the game's paths (2026-11-03) it still runs
  2,292 frames and wins 263/1000 shuffled, while the incumbent now runs
  2,291 frames, its walkers refused in the middle of a route walking on
  toward their squares' centres as the game's do: a frame faster than
  this program, both 36 s by the game's rule.  With the walks timed as
  the game's (2026-11-06) it runs 2,298 frames and wins 223/1000
  shuffled, its errands now setting out a tick after they come up, while
  the incumbent runs 2,295 frames, as its run on the game's own code
  does, which the model now follows to the end: 3 frames faster than
  this program, both 36 s by the game's rule.  With the first stride and
  the bodies' reads as the game's (2026-11-09) it runs 2,311 frames, the
  frame its last cube lands at on the game's own code, and still wins
  223/1000 shuffled, while the incumbent still runs 2,295 frames: 16
  frames faster than this program, both 36 s by the game's rule.  Since
  2026-11-22 the table's row is a 126-command program (commonnickname
  and senegrom's, timed at 36.672 s in the game by the table it comes
  from): the model runs it in 2,292 frames, that very time, and wins
  every run on both screens, 19 frames faster than this program, both
  36 s by the game's rule.  It stays queued at the maintainer's request
  as the live test of the 36 s its author reported.
- Suggested live test: run the incumbent once as the control, then this
  program a few times; capture the completion panel, the displayed speed
  and the editor size.  A completion at 36 s or faster takes the speed
  row at size 93.
- Result: _not yet tested in the game_.

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
  frames.  With the second review's fixes (2026-10-23) it wins **134/200
  plain and 146/200 shuffled**, about 61,500 frames, near the public 64%;
  a completion is kept on about 61% of attempts.  Three of the review's
  corrections each cost it some runs: the directional `if` reading its
  squares later, the look a give aimed by direction pays first, and
  lists finding workers where their bodies are.  With the game's hand-off
  (2026-10-26) it wins **152/200 plain** (756/1000), about 62,300 frames,
  a completion kept on about 75% of attempts, but only **91/200
  shuffled** (180/400; kept on about 9%): when its `giveto s` finds a
  worker below, the two meet on a tick that depends on the order they
  take their turns, which the shuffled screen redraws every frame and
  the game never does.  With givers heading for the square beside the
  receiver nearest them (2026-10-31) it wins **142/200 plain** (730/1000),
  about 61,800 frames, a completion kept on about 69% of attempts (72%
  over 1000 runs), and **93/200 shuffled** (185/400; kept on about 10%).
  With its step drawing its square as the game does (2026-11-02) it wins
  **146/200 plain** (698/1000), about 61,600 frames, a completion kept on
  about 72% of attempts (67% over 1000 runs), and **77/200 shuffled**
  (165/400; kept on about 2%).  The rise over 200 runs is the seeds: over
  8,000 runs it wins 68.5%, against 74.3% before, more of its workers fed
  to the shredders on the way.  On the game's own code, on the real
  level, the model now follows all four of the game's runs of this
  program tick for tick.  With the bodies following the game's paths
  (2026-11-03) it wins **147/200 plain** (691/1000), about 61,900
  frames, a completion kept on about 72% of attempts (65% over 1000
  runs), and **76/200 shuffled** (162/400; kept on about 2%), within
  noise of the figures before: over 8,000 plain runs it wins 68.8%,
  against 68.5%.  On the game's own code the model still follows the
  game's four runs of this program tick for tick.  With the walks timed
  as the game's (2026-11-06) it wins **146/200 plain** (691/1000), about
  61,900 frames, a completion kept on about 72% of attempts (65% over
  1000 runs), and **75/200 shuffled** (160/400; kept on about 2%),
  within noise: over 8,000 plain runs it wins 68.8%, as before.  On the
  game's own code the model now matches every worker's hands in those
  four runs too, where a feed used to let its cube go a tick or two
  late.
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
- Local emulator: 670/1000 plain and shuffled (2026-11-09; 669/1000 on
  2026-10-18, and again 2026-10-23, 2026-11-03 and 2026-11-06; 36/50
  before), average 439 frames (437 on 2026-11-06, 438 on 2026-11-03,
  within noise), a completion kept on about 61% of attempts (win/fail
  evidence only; the model's speed is an estimate, and theirs is
  live-measured).
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
  printer take is safe.  Year 21's givers died for another reason: their
  diagonal shredder give walked them onto the shredder's front, where
  their next `takefrom s` named the shredder.
- Public evidence: abfipes12 and martinez8859 report 15/25 wins (60%).
- Current emulator evidence: 111/200 plain and 120/200 shuffled
  (2026-11-13, with walkers that trade places striding and landing as
  the game's do; 223/400 and 232/400 over 400 runs; 117/200 and 106/200
  on 2026-11-09, with the first stride and the bodies' reads as the
  game's, 228/400 and 219/400 over 400 runs; 120/200 and 106/200 on
  2026-11-06, with the walks timed as the game's, 228/400 and 214/400
  over 400 runs; 125/200 and 106/200 on 2026-11-03, with the bodies
  following the game's paths, 249/400 and 208/400 over 400 runs; 115/200
  and 123/200 on 2026-11-02, with its step drawing its square as the
  game does and giving way while it waits, 240/400 and 215/400 over 400
  runs; 119/200 and 125/200 on 2026-11-01, with a take by direction at a
  printer looking before it walks to the front, 238/400 and 243/400 over
  400 runs; 131/200 and 130/200 on 2026-10-31, with takers heading for
  the square beside the partner nearest them and waiting for it, 250/400
  and 243/400 over 400 runs; 154/200 and 151/200 on 2026-10-28, with
  walkers trading places with a worker its take has pinned, 301/400 on
  both screens over 400 runs; 152/200 and 152/200 on 2026-10-26, with
  the game's hand-off: its take also takes sheets from other workers'
  hands, 301/400 and 309/400 over 400 runs; 146/200 and 133/200 on
  2026-10-23, 147/200 and 149/200 on 2026-10-18, 618/1000 and 616/1000
  on 2026-09-14), a completion kept on about 31% of attempts (32% over
  400 runs).  Over 2,000 runs the program wins 1,179 plain, against
  1,178 before (1,176 before 2026-11-09, 1,216 before 2026-11-06 and
  1,262 before 2026-11-02), a completion kept on about 41% of attempts,
  and 58.4% of 8,000 runs, against 58.3%, within noise: the trades deal
  its long runs afresh, 555 of the 8,000 lost and 565 gained, and it
  loses them in the kinds it lost before.  That 58.3% stood against
  58.4% on 2026-11-09, within noise too, and the 58.4% fell from 60.1%
  on 2026-11-06: more of its runs end in a standstill the game's own
  code shows on the real level too, a worker walking up to its partner
  bound for a square held by a worker waiting in its own take on that
  partner, and more of them crash (below).  The 60.1% fell from 64.3% on
  2026-11-02, more of its runs ending with a worker on its step, holding
  a cube, waiting for a square that a worker waiting in its own take on
  it holds, a standstill the game's own code shows on the real level
  too.  Shuffled it wins 2,125 of 4,000 runs, as before, 271 lost and
  271 gained (2,110 before 2026-11-09, 2,065 before 2026-11-06 and 2,194
  before 2026-11-03).  Winning runs average about 1,173 s (1,156 s over
  400), so they often finish only just before the game deadline.
- **Crash warning (2026-10-31, re-measured 2026-11-13), not tested by
  the maintainer:** a walker on a rising diagonal onto a cube passes
  through a third square for about ten ticks, so one square can hold two
  bodies, and this program's `takefrom nw,sw,ne` can then reach six
  things with no printer among the first five.  On the game's own code,
  on the real level, that overruns the list at the take's first tick,
  which would end the game.  This has not been run in the real game, and
  what a crash would do to the save is unknown.  How often it happens is
  a model figure: with walkers that trade places striding and landing as
  the game's do (2026-11-13) it crashes 139 of 8,000 plain runs, about 1
  in 58, within noise of the 150 before, 102 of them the runs that
  crashed before, and 56 of 4,000 shuffled, as before, so about one
  attempt in four or five over the visible run and its 25 rounds.  The
  rates above count those runs as lost, but the kept figures do not
  allow for a crash ending the attempt.  (On 2026-10-31 a model whose
  bodies followed the game's swing forecast about 1 run in 65, while the
  model then behind the rates, whose bodies did not, read about 1 in
  1,000; 7 of 8,000 plain runs on 2026-11-02, 115, about 1 in 70, on
  2026-11-03, once the bodies followed the game's paths, 150, with 52 of
  4,000 shuffled, on 2026-11-06, with the walks timed as the game's, and
  150, about 1 in 53, with 56 of 4,000 shuffled, on 2026-11-09, with the
  first stride and the bodies' reads as the game's.)  The confirmation
  only re-checks a row that is already published, so copy the save aside
  first, or skip it.  CLAUDE.md's rule against a list that can crash was
  written for one that crashes in every run; this entry stays queued
  with this warning by the maintainer's choice.
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
  641 against 199/200 at 647, an edge of about 6 seconds.  Re-measured
  2026-10-23 over 200 worlds: plain, candidate 199/200 at 607 seconds
  against the incumbent's 200/200 at 617; shuffled, 199/200 at 609
  against 198/200 at 629.  Re-measured 2026-10-26, with the printer's
  serve timed as the game's: plain, candidate 200/200 at 617 seconds
  against the incumbent's 200/200 at 625; shuffled, 200/200 at 621
  against 198/200 at 599, so under the shuffled screen the incumbent
  now runs faster.  Re-measured 2026-11-02, with list steps picking their
  squares at random as the game does: plain, candidate
  200/200 at 615 seconds against the incumbent's 200/200 at 603;
  shuffled, 200/200 at 638 against 197/200 at 634, so over 200 worlds the
  candidate now reads slower on both screens.  Over 1,000 worlds it reads
  faster on both: plain 614 against 631 seconds, shuffled 618 against
  632 (998/1000 on both screens, against 997 and 992).  The 200-world
  difference is within noise: its standard error is about 17 seconds.
  Re-measured 2026-11-03, with the bodies following the game's paths:
  over 200 worlds, plain, candidate 200/200 at 624 seconds against the
  incumbent's 198/200 at 615; shuffled, 198/200 at 623 against 199/200
  at 614.  Over 1,000 worlds, plain, 998/1000 at 620 seconds against
  995/1000 at 624; shuffled, 995/1000 at 623 against 991/1000 at 623.
  Every difference is within noise (a standard error of about 17 seconds
  over 200 worlds and 8 over 1,000).  Re-measured 2026-11-06, with the
  walks timed as the game's: over 200 worlds, plain, candidate 200/200
  at 622 seconds against the incumbent's 200/200 at 637; shuffled,
  198/200 at 628 against 199/200 at 644.  Over 1,000 worlds, plain,
  999/1000 at 613 seconds against 993/1000 at 627; shuffled, 994/1000 at
  619 against 994/1000 at 632.  It now reads faster on both screens over
  200 and over 1,000 worlds, by 13 to 16 seconds, each time within two
  standard errors (about 18 seconds over 200 worlds and 8 over 1,000).
  Re-measured 2026-11-09, with the first stride and the bodies' reads as
  the game's: over 200 worlds, plain, candidate 199/200 at 608 seconds
  against the incumbent's 199/200 at 619; shuffled, 199/200 at 620
  against 199/200 at 627.  Over 1,000 worlds, plain, 997/1000 at 615
  seconds against 993/1000 at 616; shuffled, 999/1000 at 616 against
  998/1000 at 620.  It still reads faster on both screens over 200 and
  over 1,000 worlds, now by 1 to 12 seconds, each time within one
  standard error: the size-5 row now runs about 11 seconds faster over
  1,000 plain worlds, the candidate about where it was.  Re-measured
  2026-11-13, with walkers that trade places striding and landing as the
  game's do: over 200 worlds, plain, candidate 200/200 at 609 seconds
  against the incumbent's 199/200 at 623; shuffled, 199/200 at 628
  against 199/200 at 630.  Over 1,000 worlds, plain, 998/1000 at 615
  seconds against 993/1000 at 618; shuffled, 999/1000 at 616 against
  996/1000 at 622.  It still reads faster on both screens over 200 and
  over 1,000 worlds, now by 2 to 15 seconds, each time within one
  standard error.  It stays queued for now by the maintainer's choice
  (2026-11-02).
- Suggested live test: five alternating fresh runs of the incumbent and this
  candidate; compare medians and timeout count.
- Result: _not yet tested in the game_.

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
  Re-measured 2026-10-23: incumbent 509, size 21 431, size 19 356 plain
  (359 shuffled), all reading 15 s by the game's rule, so the live
  control decides.  In the model a completion is kept about once in 5
  attempts for the incumbent, once in 17 at size 21 and once in 95 at
  size 19.
- Suggested live test: the size-21 fallback first, repeated ~14-second
  runs until the game keeps a completion (back to the elevator after
  every failed round), then the size 19; capture the displayed speed and
  editor size.
- Result: _not yet tested in the game_.

## Parked long shots (kept less than once in 100 attempts)

The game keeps a completion only when at least 14 of the 25 random
rounds it plays after a visible win also win, so under CLAUDE.md's floor
a candidate needs about a 35% win rate per run; below that, one kept
completion costs more than 100 attempts (about 4,400 at 25% and 650
million at 10%).  Nothing here clears it.  First come the low-percent
leads (valid programs for the SolutionsLowPercent tier, at about 1% to
25% per run), then a speed ladder that cannot be settled, then the
long shots below 1 in 100 runs.  All stay here with their paste-ready
files in case the rule changes or a higher-rate variant turns up.
Nothing below this line needs game time.

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
  175/1000 shuffled, averaging 399; 24 item actions to its 27.
  Re-measured 2026-11-02: 117/1000 plain, every win still at 370 frames,
  and 121/1000 shuffled, averaging 399.  Its two workers take their
  steps up on one tick, and as in the game both now find the square
  free, so the second draws its other square half the time; the model
  used to count the first one's claim at once.  Canonical sizes are 17
  and 20; the incumbent's authoritative live score is 5.
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
  the same 411 frames and 11 item actions.  Re-measured 2026-11-03: both
  still win the same 322/1000 runs on both screens, each run in the same
  frames, but a win now takes 414-421 frames, 417 on average (6 s by the
  game's rule), with 11 item actions; and on 2026-11-06, with the walks
  timed as the game's, they still win the same runs, each in the same
  frames on both programs, a win in 412-420 frames, 416 on average,
  still 6 s.  Canonical sizes are 83 and 84.
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
  Re-measured 2026-11-02: **65/1000 plain and 85/1000 shuffled**,
  winning runs about 20 s by the game's rule, from 18; over 2,000 plain
  runs 130 against 149, within noise.  Re-measured 2026-11-09, with a
  walker's fall into a hole beginning as the game's does: **72/1000
  plain and 76/1000 shuffled**, winning runs about 19 s by the game's
  rule; over 2,000 plain runs 139 against 130, within noise.
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
  shots; 11/1000 and 15/1000 on 2026-11-02, and 9/1000 and 12/1000 on
  2026-11-09, with a walker's fall into a hole beginning as the game's
  does (22 of 2,000 plain runs against 26, within noise).  The model now
  wins the public size-4 program too (72/1000 plain, 76/1000 shuffled;
  65 and 85 on 2026-11-02).  The figure first given here, `7! / 7^7`
  (about 0.612%) for value uniqueness, assumed independent draws.
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
  33/200 plain and 30/200 shuffled, winning runs averaging about 437 s;
  on 2026-11-02 40/200 plain and 42/200 shuffled, about 444 s plain
  (385 of 2,000 plain runs against 346, within noise); and on 2026-11-03
  39/200 on both screens, about 450 s plain (383 of 2,000 plain runs
  against 385, within noise).
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
  averaging 1,018 frames (about 16 s); 84/1,000 before.  Re-measured
  2026-11-03: still 93/1,000 plain and 95/1,000 shuffled, the winning
  runs now averaging 1,033 frames plain (16 s by the game's rule) and
  1,064 shuffled (17 s).  Every win completes far before the deadline.
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

### [ ] Year 67 - Decimal Doubler - speed tie-break at size 205 (fallbacks 206-209)

- **Paste-ready program:** [SolutionsToTry/Year 67 - Decimal Doubler - live-only speed tie-break at size 205.txt](<SolutionsToTry/Year 67 - Decimal Doubler - live-only speed tie-break at size 205.txt>)
- **Fallback (size 206):** [SolutionsToTry/Year 67 - Decimal Doubler - fallback size 206.txt](<SolutionsToTry/Year 67 - Decimal Doubler - fallback size 206.txt>)
- **Fallback (size 207):** [SolutionsToTry/Year 67 - Decimal Doubler - fallback size 207.txt](<SolutionsToTry/Year 67 - Decimal Doubler - fallback size 207.txt>)
- **Fallback (size 208):** [SolutionsToTry/Year 67 - Decimal Doubler - fallback size 208.txt](<SolutionsToTry/Year 67 - Decimal Doubler - fallback size 208.txt>)
- **Fallback (size 209):** [SolutionsToTry/Year 67 - Decimal Doubler - conservative fallback size 209.txt](<SolutionsToTry/Year 67 - Decimal Doubler - conservative fallback size 209.txt>)
- Goal: retain the displayed speed record of 41 while reducing the
  secondary size from 210 commands to 205.
- Exact edits: after opening `pickup ne; step n`, delete all three consecutive
  `mem2 = set c` commands; in loop `j`, after `step mem3; step e`, delete both
  consecutive `tell everyone hi` commands.  The ladder removes one of them
  per rung: 209 deletes one tell, 208 one store as well, 207 the second
  tell, 206 a second store, and 205 the third.
- Why it is parked (2026-10-20): the two tells are the presser's clock, not
  padding.  The program has no `listenfor`, but the tells make loop `j`
  last exactly 127 frames, the length of the writers' digit cycle, and the
  pads show the full number for only 11 frames of each cycle.  With one
  tell gone the presser's lap is 85 frames (43 with both gone), so most
  presses read part-written or empty pads; Decimal Doubler counts a press
  only when it reads the next doubling, and any other reading starts the
  count again.  Every rung deletes the first tell.  On the game's own code
  the 209 rung fails this way, and the review's corrected model wins the
  incumbent 20 times in 20 and every rung in none of 20.
- Suggested live test: none.  Screen any future Year 67 speed edit on the
  corrected model first.
- Result: _not tested in the game_.

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
  Re-measured 2026-11-03: still exactly 1/69,510, at that seed, in 342
  frames, which by the game's rule (16 ms a tick, whole seconds) shows 5
  s.
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
  Re-measured 2026-11-03: that seed still wins, in 2,099 frames with 172
  item actions, which by the game's rule (16 ms a tick, whole seconds)
  shows 33 s.
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
  group.  An isolated seed-offset emulator run then won in 310 frames
  with displayed speed 5 and 14 item actions.  Re-measured 2026-11-03:
  that seed still wins, with 14 item actions, but now in 718 frames,
  11 s by the game's rule (16 ms a tick, whole seconds); and on
  2026-11-06, with the walks timed as the game's, in 704 frames, still
  11 s.
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
  frames with displayed speed 3.  Re-measured 2026-11-03: still
  44/10,000, every win in 146 frames, which by the game's rule (16 ms a
  tick, whole seconds) shows 2 s.
- Expected editor size: **5**; all commands are available in Year 33.
- Suggested live test: repeated quick runs; roughly a few hundred attempts per
  observed win is plausible, but record the actual tie behavior.
- Result: _not yet tested locally in the game_.
