from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
from check_readme import solution_size

year_re = re.compile(r"^Year (4[89]|5[0-9]|6[0-8]) ")

for base, label in [
    (ROOT / "Solutions99+", "current99"),
    (ROOT / "Solutions50+", "current50"),
    (ROOT / "SolutionsLowPercent", "currentLow"),
    (ROOT / ".codex_external_abfipes", "external"),
]:
    for path in sorted(base.rglob("*")):
        if not path.is_file() or not year_re.match(path.name):
            continue
        rel = path.relative_to(ROOT)
        lines = path.read_text(encoding="utf-8-sig", errors="replace").splitlines()
        credit = " | ".join(line.strip() for line in lines[:6] if " by " in line or "failed " in line)
        print(f"{label}\t{solution_size(path)}\t{rel}\t{credit}")
