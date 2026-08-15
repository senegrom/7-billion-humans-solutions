from __future__ import annotations

import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))

from check_readme import solution_size


GIT_PATH = "Solutions99+/Year 66 - Decimal Counter (speed).txt"
OUT = Path(__file__).resolve().parent


def head_lines() -> list[str]:
    completed = subprocess.run(
        ["git", "show", f"HEAD:{GIT_PATH}"],
        cwd=ROOT,
        check=True,
        capture_output=True,
        text=True,
        encoding="utf-8",
    )
    return completed.stdout.splitlines()


def write_variant(name: str, removals: range | tuple[int, ...]) -> Path:
    lines = head_lines()
    removal_set = set(removals)
    path = OUT / name
    path.write_text(
        "\n".join(line for lineno, line in enumerate(lines, 1) if lineno not in removal_set)
        + "\n",
        encoding="utf-8",
        newline="\n",
    )
    return path


def main() -> None:
    baseline = head_lines()
    assert baseline[76:80] == ["\t\t\tmem1 = set myitem"] * 4
    assert baseline[121:124] == ["\t\t\tmem1 = set myitem"] * 3
    assert baseline[190:204] == ["\t\t\t\t\tmem1 = set myitem"] * 14

    baseline_path = write_variant("y66_speed_head_baseline.txt", ())
    paths = [
        write_variant("y66_speed_drop_block77_80.txt", range(77, 81)),
        write_variant("y66_speed_drop_block122_124.txt", range(122, 125)),
        write_variant("y66_speed_drop_one_at204.txt", (204,)),
        write_variant("y66_speed_drop_block191_204.txt", range(191, 205)),
        write_variant(
            "y66_speed_drop_blocks77_80_and122_124.txt",
            tuple(range(77, 81)) + tuple(range(122, 125)),
        ),
        write_variant(
            "y66_speed_drop_blocks77_80_122_124_191_204.txt",
            tuple(range(77, 81)) + tuple(range(122, 125)) + tuple(range(191, 205)),
        ),
        write_variant(
            "y66_speed_proven_62_67_and105.txt",
            tuple(range(62, 68)) + (105,),
        ),
        write_variant(
            "y66_speed_proven_plus_block77_80.txt",
            tuple(range(62, 68)) + (105,) + tuple(range(77, 81)),
        ),
        write_variant(
            "y66_speed_proven_plus_block122_124.txt",
            tuple(range(62, 68)) + (105,) + tuple(range(122, 125)),
        ),
        write_variant(
            "y66_speed_proven_plus_one_at204.txt",
            tuple(range(62, 68)) + (105, 204),
        ),
        write_variant(
            "y66_speed_proven_plus_blocks77_80_and122_124.txt",
            tuple(range(62, 68)) + (105,) + tuple(range(77, 81)) + tuple(range(122, 125)),
        ),
        write_variant(
            "y66_speed_size239_plus_singleton95.txt",
            tuple(range(62, 68))
            + (95, 105)
            + tuple(range(77, 81))
            + tuple(range(122, 125)),
        ),
        write_variant(
            "y66_speed_size239_plus_singleton108.txt",
            tuple(range(62, 68))
            + (105, 108)
            + tuple(range(77, 81))
            + tuple(range(122, 125)),
        ),
        write_variant(
            "y66_speed_proven_plus_block77_80_plus_one_at204.txt",
            tuple(range(62, 68)) + (105, 204) + tuple(range(77, 81)),
        ),
        write_variant(
            "y66_speed_proven_plus_blocks77_80_122_124_191_204.txt",
            tuple(range(62, 68))
            + (105,)
            + tuple(range(77, 81))
            + tuple(range(122, 125))
            + tuple(range(191, 205)),
        ),
    ]
    print(f"{baseline_path.name}: canonical size {solution_size(baseline_path)}")
    for path in paths:
        print(f"{path.name}: canonical size {solution_size(path)}")


if __name__ == "__main__":
    main()
