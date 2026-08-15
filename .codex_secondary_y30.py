from __future__ import annotations

import itertools
import os
from pathlib import Path
import re
import subprocess
import tempfile


ROOT = Path(r"E:\OneDrive\CodingChatGPT\7bh")
EMU = ROOT / "emulator" / "emu.exe"
LEVEL = ROOT / "tools" / "extract" / "out" / "levels" / "30_fill_the_floor.lvl"
DIRECTIONS = ("nw", "w", "sw", "n", "s", "ne", "e", "se")
VECTORS = {
    "nw": (-1, -1), "w": (-1, 0), "sw": (-1, 1), "n": (0, -1),
    "s": (0, 1), "ne": (1, -1), "e": (1, 0), "se": (1, 1),
}


def candidate(directions: tuple[str, ...]) -> str:
    return (
        "-- 30: Fill the Floor --\n\n"
        "mem1 = nearest printer\n"
        "a:\n"
        "takefrom mem1\n"
        f"step {','.join(directions)}\n"
        "drop\n"
        "jump a\n"
    )


def run(path: Path, trials: int, cap: int, timeout: int = 60) -> tuple[int, float, str]:
    env = os.environ.copy()
    env["EMU_CAP"] = str(cap)
    proc = subprocess.run(
        [str(EMU), str(LEVEL), str(path), str(trials)],
        cwd=ROOT,
        env=env,
        capture_output=True,
        text=True,
        timeout=timeout,
    )
    wins_match = re.search(r"trials\s+:\s+\d+, wins (\d+)", proc.stdout)
    speed_match = re.search(r"speed\s+:\s+([0-9.]+)", proc.stdout)
    wins = int(wins_match.group(1)) if wins_match else 0
    speed = float(speed_match.group(1)) if speed_match else float("inf")
    return wins, speed, proc.stdout


def main() -> None:
    with tempfile.TemporaryDirectory(prefix="codex-y30-directions-") as tmp:
        temp = Path(tmp)
        survivors: list[tuple[float, tuple[str, ...], Path]] = []
        tested = 0
        for count in range(1, len(DIRECTIONS) + 1):
            for directions in itertools.combinations(DIRECTIONS, count):
                xs = [VECTORS[d][0] for d in directions]
                ys = [VECTORS[d][1] for d in directions]
                if not (min(xs) < 0 < max(xs) and min(ys) < 0 < max(ys)):
                    continue
                tested += 1
                path = temp / ("_".join(directions) + ".txt")
                path.write_text(candidate(directions), encoding="utf-8")
                wins, speed, _ = run(path, trials=1, cap=50_000, timeout=20)
                if wins:
                    survivors.append((speed, directions, path))
                    print(f"WIN {speed:7.1f} {','.join(directions)}", flush=True)
                if tested % 16 == 0:
                    print(f"PROGRESS tested={tested} survivors={len(survivors)}", flush=True)

        print("\nTOP SCREEN", flush=True)
        for speed, directions, _ in sorted(survivors)[:20]:
            print(f"{speed:8.1f} {','.join(directions)}", flush=True)

        print("\nVALIDATION", flush=True)
        for _, directions, path in sorted(survivors)[:12]:
            wins, speed, output = run(path, trials=30, cap=400_000, timeout=180)
            print(f"VALIDATE {wins:3}/30 speed={speed:8.1f} {','.join(directions)}", flush=True)
            if wins == 30:
                print("TEXT", flush=True)
                print(path.read_text(encoding="utf-8"), flush=True)
                print(output, flush=True)


if __name__ == "__main__":
    main()
