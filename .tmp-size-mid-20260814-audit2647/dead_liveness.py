from __future__ import annotations

import re
import sys
from dataclasses import dataclass
from pathlib import Path


MEM_RE = re.compile(r"\bmem([1-4])\b")
ASSIGN_RE = re.compile(r"^\s*(mem[1-4])\s*=\s*(?!=)(.*)$")
LABEL_RE = re.compile(r"^\s*([A-Za-z][A-Za-z0-9_]*)\s*:\s*$")


@dataclass
class Node:
    line: int
    text: str
    kind: str
    uses: set[str]
    defs: set[str]
    succ: set[int]


def parse(path: Path) -> list[Node]:
    source = path.read_text(encoding="utf-8-sig").splitlines()
    nodes: list[Node] = []
    i = 0
    while i < len(source):
        raw = source[i]
        stripped = raw.strip()
        if stripped.startswith("DEFINE COMMENT"):
            break
        if not stripped or stripped.startswith("--"):
            i += 1
            continue
        start = i + 1
        if stripped.startswith("if "):
            parts = [stripped]
            while not parts[-1].endswith(":"):
                i += 1
                if i >= len(source):
                    raise ValueError(f"unterminated if at {path}:{start}")
                parts.append(source[i].strip())
            text = " ".join(parts)
            kind = "if"
        else:
            text = stripped
            if text in {"else", "else:"}:
                kind = "else"
            elif text in {"endif", "endif:"}:
                kind = "endif"
            elif LABEL_RE.fullmatch(text):
                kind = "label"
            elif text.startswith("jump "):
                kind = "jump"
            elif text == "end":
                kind = "end"
            else:
                kind = "op"
        assignment = ASSIGN_RE.match(text)
        defs: set[str] = set()
        uses_text = text
        if assignment:
            defs.add(assignment.group(1))
            uses_text = assignment.group(2)
        uses = {f"mem{m}" for m in MEM_RE.findall(uses_text)}
        nodes.append(Node(start, text, kind, uses, defs, set()))
        i += 1

    labels: dict[str, int] = {}
    for idx, node in enumerate(nodes):
        if node.kind == "label":
            labels[LABEL_RE.fullmatch(node.text).group(1)] = idx

    stack: list[tuple[int, int | None]] = []
    match: dict[int, tuple[int | None, int]] = {}
    for idx, node in enumerate(nodes):
        if node.kind == "if":
            stack.append((idx, None))
        elif node.kind == "else":
            if_idx, prior_else = stack.pop()
            if prior_else is not None:
                raise ValueError(f"double else at {path}:{node.line}")
            stack.append((if_idx, idx))
        elif node.kind == "endif":
            if_idx, else_idx = stack.pop()
            match[if_idx] = (else_idx, idx)
            if else_idx is not None:
                match[else_idx] = (None, idx)
    if stack:
        raise ValueError(f"unclosed if in {path}: {stack}")

    for idx, node in enumerate(nodes):
        next_idx = idx + 1
        if node.kind == "if":
            else_idx, end_idx = match[idx]
            if next_idx < len(nodes):
                node.succ.add(next_idx)
            false_idx = (else_idx + 1) if else_idx is not None else (end_idx + 1)
            if false_idx < len(nodes):
                node.succ.add(false_idx)
        elif node.kind == "else":
            _, end_idx = match[idx]
            after = end_idx + 1
            if after < len(nodes):
                node.succ.add(after)
        elif node.kind == "jump":
            label = node.text.split(maxsplit=1)[1].strip()
            if label not in labels:
                raise ValueError(f"unknown label {label!r} at {path}:{node.line}")
            node.succ.add(labels[label])
        elif node.kind != "end" and next_idx < len(nodes):
            node.succ.add(next_idx)
    return nodes


def analyze(path: Path) -> None:
    nodes = parse(path)
    reachable: set[int] = set()
    pending = [0] if nodes else []
    while pending:
        idx = pending.pop()
        if idx in reachable:
            continue
        reachable.add(idx)
        pending.extend(nodes[idx].succ - reachable)
    live_in = [set() for _ in nodes]
    live_out = [set() for _ in nodes]
    changed = True
    while changed:
        changed = False
        for idx in range(len(nodes) - 1, -1, -1):
            node = nodes[idx]
            new_out = set().union(*(live_in[s] for s in node.succ)) if node.succ else set()
            new_in = node.uses | (new_out - node.defs)
            if new_out != live_out[idx] or new_in != live_in[idx]:
                live_out[idx] = new_out
                live_in[idx] = new_in
                changed = True
    dead = []
    for idx, node in enumerate(nodes):
        if node.defs and node.defs.isdisjoint(live_out[idx]):
            dead.append((node.line, node.text, sorted(live_out[idx]), sorted(node.succ)))
    if dead:
        print(path)
        for line, text, live, succ in dead:
            print(f"  {line}: {text} | live-out={live} succ={succ}")
    unreachable = [node for idx, node in enumerate(nodes) if idx not in reachable]
    if unreachable:
        if not dead:
            print(path)
        print("  unreachable:")
        for node in unreachable:
            print(f"    {node.line}: {node.text}")


for arg in sys.argv[1:]:
    analyze(Path(arg))
