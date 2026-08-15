from __future__ import annotations

import os
from pathlib import Path
import re
import subprocess
import tempfile


ROOT = Path(r"E:\OneDrive\CodingChatGPT\7bh")
EMU = ROOT / "emulator" / "emu.exe"
LEVEL = ROOT / "tools" / "extract" / "out" / "levels" / "18_uniquely_disposed.lvl"
SOURCE = ROOT / "Solutions99+" / "Year 18 - Uniquely Disposed (speed).txt"


def code_lines() -> list[str]:
    return [
        line
        for line in SOURCE.read_text(encoding="utf-8").splitlines()
        if line.strip() and not line.lstrip().startswith("--")
    ]


def run(path: Path, trials: int = 1) -> tuple[int, float, str]:
    env = os.environ.copy()
    env["EMU_CAP"] = "100000"
    proc = subprocess.run(
        [str(EMU), str(LEVEL), str(path), str(trials)],
        cwd=ROOT,
        env=env,
        capture_output=True,
        text=True,
        timeout=60,
    )
    wins_match = re.search(r"trials\s+:\s+\d+, wins (\d+)", proc.stdout)
    speed_match = re.search(r"speed\s+:\s+([0-9.]+)", proc.stdout)
    return (
        int(wins_match.group(1)) if wins_match else 0,
        float(speed_match.group(1)) if speed_match else float("inf"),
        proc.stdout,
    )


def main() -> None:
    original = code_lines()
    candidates: dict[str, str] = {}

    for index, line in enumerate(original):
        changed = original[:index] + original[index + 1 :]
        text = "\n".join(changed) + "\n"
        candidates.setdefault(text, f"delete {index + 1}:{line}")

    directions = ("s", "sw", "se", "e", "w")
    for index, line in enumerate(original):
        if not line.startswith("step "):
            continue
        old = line.split(maxsplit=1)[1]
        for direction in directions:
            if direction == old:
                continue
            changed = original.copy()
            changed[index] = f"step {direction}"
            text = "\n".join(changed) + "\n"
            candidates.setdefault(text, f"replace {index + 1}:{old}->{direction}")

    with tempfile.TemporaryDirectory(prefix="codex-y18-speed-") as tmp:
        temp = Path(tmp)
        winners: list[tuple[float, str, Path]] = []
        for number, (text, label) in enumerate(candidates.items(), 1):
            path = temp / f"candidate-{number:03}.txt"
            path.write_text(text, encoding="utf-8")
            wins, speed, _ = run(path)
            if wins:
                winners.append((speed, label, path))
                print(f"WIN speed={speed} {label}", flush=True)

        print("TOP", flush=True)
        for speed, label, path in sorted(winners)[:20]:
            wins, validated_speed, output = run(path, trials=100)
            print(f"VALIDATE {wins}/100 speed={validated_speed} {label}", flush=True)
            if wins == 100:
                print(path.read_text(encoding="utf-8"), flush=True)
                print(output, flush=True)


if __name__ == "__main__":
    main()
