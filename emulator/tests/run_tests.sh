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
  bash build.sh
  emu=./emu.exe
  # a fresh build can be briefly locked by scanners -- wait until runnable
  for _ in 1 2 3 4 5 6 7 8 9 10; do
    "$emu" >/dev/null 2>&1
    [ $? -ne 126 ] && break
    sleep 1
  done
fi

pass=0; fail=0

# expect_win <level> <solution> <expected_size>
expect_win() {
  out=$("$emu" "$1" "$2" 2>&1); rc=$?
  size=$(printf '%s\n' "$out" | sed -n 's/^size *: *\([0-9]*\).*/\1/p')
  if [ "$rc" -eq 0 ] && [ "$size" = "$3" ]; then
    echo "PASS win  $(basename "$2") -> size $size"; pass=$((pass+1))
  else
    echo "FAIL win  $(basename "$2") (rc=$rc size=$size want $3)"; echo "$out"; fail=$((fail+1))
  fi
}

# expect_fail <level> <solution>: a broken program must not win
expect_fail() {
  "$emu" "$1" "$2" >/dev/null 2>&1
  if [ $? -ne 0 ]; then echo "PASS lose $(basename "$2")"; pass=$((pass+1))
  else echo "FAIL lose $(basename "$2") (unexpected win)"; fail=$((fail+1)); fi
}

expect_win  levels/year03_transport.lvl tests/year03.txt   5
expect_win  levels/year14_shredding.lvl tests/year14.txt   4
expect_win  levels/decision_demo.lvl    tests/decision.txt 5
expect_win  levels/else_colon.lvl       tests/else_colon.txt 5
expect_win  levels/swap_sort.lvl        tests/swap_sort.txt  6
expect_win  levels/printer_take.lvl     tests/printer_take.txt 4

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
