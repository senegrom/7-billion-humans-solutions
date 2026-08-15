from __future__ import annotations

import os
from pathlib import Path
import re
import subprocess
import tempfile


ROOT = Path(r"E:\OneDrive\CodingChatGPT\7bh")
EMU = ROOT / "emulator" / "emu.exe"
LEVEL = ROOT / "tools" / "extract" / "out" / "levels" / "11_injection_sites_1.lvl"
SOURCE = ROOT / "Solutions99+" / "Year 11 - Injection Sites 1 (speed).txt"


def source_lines() -> list[str]:
    return [
        line
        for line in SOURCE.read_text(encoding="utf-8").splitlines()
        if line.strip() and not line.lstrip().startswith("--")
    ]


def run(path: Path, trials: int = 1) -> tuple[int, float, int, str]:
    env = os.environ.copy()
    env["EMU_CAP"] = "50000"
    proc = subprocess.run(
        [str(EMU), str(LEVEL), str(path), str(trials)],
        cwd=ROOT,
        env=env,
        capture_output=True,
        text=True,
        timeout=30,
    )
    wins_match = re.search(r"trials\s+:\s+\d+, wins (\d+)", proc.stdout)
    speed_match = re.search(r"speed\s+:\s+([0-9.]+)", proc.stdout)
    size_match = re.search(r"size\s+:\s+(\d+)", proc.stdout)
    return (
        int(wins_match.group(1)) if wins_match else 0,
        float(speed_match.group(1)) if speed_match else float("inf"),
        int(size_match.group(1)) if size_match else 999,
        proc.stdout,
    )


def main() -> None:
    original = source_lines()
    candidates: dict[str, str] = {}

    for index, line in enumerate(original):
        changed = original[:index] + original[index + 1 :]
        candidates.setdefault("\n".join(changed) + "\n", f"delete physical {index + 1}:{line.strip()}")

    directions = ("n", "s", "ne", "se", "sw")
    for index, line in enumerate(original):
        stripped = line.strip()
        if not stripped.startswith("step "):
            continue
        old = stripped.split(maxsplit=1)[1]
        for direction in directions:
            if direction == old:
                continue
            changed = original.copy()
            prefix = line[: len(line) - len(line.lstrip())]
            changed[index] = f"{prefix}step {direction}"
            candidates.setdefault("\n".join(changed) + "\n", f"replace {index + 1}:{old}->{direction}")

    with tempfile.TemporaryDirectory(prefix="codex-y11-speed-") as tmp:
        temp = Path(tmp)
        winners: list[tuple[float, int, str, Path]] = []
        for number, (text, label) in enumerate(candidates.items(), 1):
            path = temp / f"candidate-{number:03}.txt"
            path.write_text(text, encoding="utf-8")
            wins, speed, size, _ = run(path)
            if wins:
                winners.append((speed, size, label, path))
                print(f"WIN speed={speed} size={size} {label}", flush=True)

        print("TOP", flush=True)
        for _, _, label, path in sorted(winners)[:20]:
            wins, speed, size, output = run(path, trials=100)
            print(f"VALIDATE {wins}/100 speed={speed} size={size} {label}", flush=True)
            if wins == 100:
                print(path.read_text(encoding="utf-8"), flush=True)
                print(output, flush=True)


if __name__ == "__main__":
    main()
