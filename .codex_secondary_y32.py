from __future__ import annotations

import itertools
import os
from pathlib import Path
import re
import subprocess
import tempfile


ROOT = Path(r"E:\OneDrive\CodingChatGPT\7bh")
EMU = ROOT / "emulator" / "emu.exe"
LEVEL = ROOT / "tools" / "extract" / "out" / "levels" / "32_creative_writhing.lvl"
DIRECTIONS = ("nw", "sw", "n", "s", "ne", "se")


def candidate(directions: tuple[str, ...]) -> str:
    return (
        "-- 32: Creative Writhing --\n\n"
        "a:\n"
        f"step {','.join(directions)}\n"
        "pickup c\n"
        "write 99\n"
        "drop\n"
        "jump a\n"
    )


def run(path: Path, trials: int, cap: int) -> tuple[bool, float, int]:
    env = os.environ.copy()
    env["EMU_CAP"] = str(cap)
    proc = subprocess.run(
        [str(EMU), str(LEVEL), str(path), str(trials)],
        cwd=ROOT,
        env=env,
        capture_output=True,
        text=True,
        timeout=120,
    )
    wins_match = re.search(r"trials\s+:\s+\d+, wins (\d+)", proc.stdout)
    speed_match = re.search(r"speed\s+:\s+([0-9.]+)", proc.stdout)
    wins = int(wins_match.group(1)) if wins_match else 0
    speed = float(speed_match.group(1)) if speed_match else float("inf")
    return proc.returncode == 0, speed, wins


def main() -> None:
    with tempfile.TemporaryDirectory(prefix="codex-y32-secondary-") as tmp:
        temp = Path(tmp)
        screened: list[tuple[float, tuple[str, ...], Path]] = []
        for count in range(1, len(DIRECTIONS) + 1):
            for dirs in itertools.combinations(DIRECTIONS, count):
                path = temp / ("_".join(dirs) + ".txt")
                path.write_text(candidate(dirs), encoding="utf-8")
                all_win, speed, wins = run(path, trials=1, cap=80_000)
                print(f"SCREEN {','.join(dirs):20} wins={wins}/1 speed={speed}", flush=True)
                if all_win:
                    screened.append((speed, dirs, path))

        print("\nTOP SCREEN", flush=True)
        for speed, dirs, _ in sorted(screened)[:12]:
            print(f"{speed:8.1f} {','.join(dirs)}", flush=True)

        print("\nVALIDATION", flush=True)
        for _, dirs, path in sorted(screened)[:12]:
            all_win, speed, wins = run(path, trials=50, cap=400_000)
            print(
                f"VALIDATE {','.join(dirs):20} wins={wins}/50 speed={speed} all={all_win}",
                flush=True,
            )


if __name__ == "__main__":
    main()
