#!/usr/bin/env bash
# Regression tests: each known-good solution must WIN its level, and a mutated
# (broken) program must FAIL. Proves the VM discriminates good from bad.
set -uo pipefail
cd "$(dirname "$0")/.."

# EMU_BIN overrides the binary (useful when AV/sync scanners lock fresh builds
# inside synced folders); otherwise build and run in place.
if [ -n "${EMU_BIN:-}" ]; then
  emu=$EMU_BIN
else
  if ! bash build.sh; then
    echo "FAIL build"
    exit 1
  fi
  emu=./emu.exe
  # a fresh build can be briefly locked by scanners -- wait until runnable
  for _ in 1 2 3 4 5 6 7 8 9 10; do
    "$emu" >/dev/null 2>&1
    [ $? -ne 126 ] && break
    sleep 1
  done
fi

pass=0; fail=0

# expect_win <level> <solution> <expected_size> [expected_rounds]
expect_win() {
  out=$("$emu" "$1" "$2" 2>&1); rc=$?
  size=$(printf '%s\n' "$out" | sed -n 's/^size *: *\([0-9]*\).*/\1/p')
  rounds=$(printf '%s\n' "$out" | sed -n 's/^rounds *: *\([0-9]*\)\.\..*/\1/p')
  if [ "$rc" -eq 0 ] && [ "$size" = "$3" ] \
      && { [ -z "${4:-}" ] || [ "$rounds" = "$4" ]; }; then
    detail="size $size"
    [ -n "${4:-}" ] && detail="$detail, rounds $rounds"
    echo "PASS win  $(basename "$2") -> $detail"; pass=$((pass+1))
  else
    echo "FAIL win  $(basename "$2") (rc=$rc size=$size/$3 rounds=$rounds/${4:--})"
    echo "$out"; fail=$((fail+1))
  fi
}

# expect_fail <level> <solution> [frame-cap]: a broken program must not win.
# A capped check also fixes trials at one so deliberate live-lock controls stay
# quick; each emulator invocation is a fresh process, so EMU_CAP cannot leak.
expect_fail() {
  if [ -n "${3:-}" ]; then
    EMU_CAP="$3" "$emu" "$1" "$2" 1 >/dev/null 2>&1; rc=$?
  else
    "$emu" "$1" "$2" >/dev/null 2>&1; rc=$?
  fi
  if [ "$rc" -ne 0 ]; then echo "PASS lose $(basename "$2")"; pass=$((pass+1))
  else echo "FAIL lose $(basename "$2") (unexpected win)"; fail=$((fail+1)); fi
}

expect_win  levels/year03_transport.lvl tests/year03.txt   5
expect_win  levels/year14_shredding.lvl tests/year14.txt   4
expect_win  levels/decision_demo.lvl    tests/decision.txt 5
expect_win  levels/else_colon.lvl       tests/else_colon.txt 5
expect_win  levels/swap_sort.lvl        tests/swap_sort.txt  6
expect_win  levels/printer_take.lvl     tests/printer_take.txt 6
expect_win  levels/printer_queue.lvl    tests/printer_queue.txt 4 154
expect_win  levels/finished_intent.lvl  tests/finished_intent.txt 4 119
expect_win  levels/shred_min_held.lvl   tests/shred_min_held.txt 5 56

# Holding unrelated cubes is allowed, but the first shred must still be the
# room minimum.  Feeding the 2 instead of the 1 must never complete the goal.
expect_fail levels/shred_min_held.lvl tests/shred_min_wrong.txt 1000

# A customer whose next step points into the printer stays at the front.  It
# must not become transparent merely because the next opcode says "step".
expect_fail levels/printer_queue.lvl tests/printer_queue_blocked.txt 1000

# A customer which did step out is solid on its newly claimed tile.  If the
# machine handoff makes the whole body transparent, the follower overlaps it
# and wrongly feeds a second cube to the shredder.
expect_fail levels/printer_queue_solid.lvl tests/printer_queue_solid.txt 1000

# Negative control: drop the delivery step -> cube never reaches the pad.
printf 'step s\npickup c\nstep s\n' > tests/_broken.txt
expect_fail levels/year03_transport.lvl tests/_broken.txt
rm -f tests/_broken.txt

# Negative control: swap in the wrong direction -> row never sorts.
printf 'pickup s\na:\nif myitem < e:\n\tstep e\nendif\njump a\n' > tests/_broken.txt
expect_fail levels/swap_sort.lvl tests/_broken.txt
rm -f tests/_broken.txt

echo "----"
echo "passed $pass, failed $fail"
[ "$fail" -eq 0 ]
