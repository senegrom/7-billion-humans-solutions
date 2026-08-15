import numpy as np

MASK = 0xFFFFFFFF
MUL = np.uint32(2654435761)
ADD = np.uint32(12345)


def xr_scalar(x: int) -> int:
    x ^= (x << 13) & MASK
    x ^= x >> 17
    x ^= (x << 5) & MASK
    return x & MASK


def advance_scalar(x: int, n: int) -> int:
    for _ in range(n):
        x = xr_scalar(x)
    return x


TABLE = np.empty((4, 256), dtype=np.uint32)
for byte in range(4):
    for value in range(256):
        TABLE[byte, value] = advance_scalar(value << (8 * byte), 72)


def after_constructors(x: np.ndarray) -> np.ndarray:
    return (
        TABLE[0, x & np.uint32(255)]
        ^ TABLE[1, (x >> np.uint32(8)) & np.uint32(255)]
        ^ TABLE[2, (x >> np.uint32(16)) & np.uint32(255)]
        ^ TABLE[3, x >> np.uint32(24)]
    )


def xr(x: np.ndarray) -> np.ndarray:
    x ^= x << np.uint32(13)
    x ^= x >> np.uint32(17)
    x ^= x << np.uint32(5)
    return x


def sums(state: np.ndarray, n: int) -> tuple[np.ndarray, np.ndarray]:
    total = np.zeros(state.size, dtype=np.uint16)
    for _ in range(n):
        xr(state)
        total += (state % np.uint32(10)).astype(np.uint16)
    return state, total


def exact_values(seed: int) -> list[int]:
    x = ((seed * 2654435761 + 12345) & MASK) | 1
    for _ in range(72):
        x = xr_scalar(x)
    values = []
    for _ in range(36):
        x = xr_scalar(x)
        values.append(x % 10)
    return values


CHUNK = 5_000_000
LIMIT = 2**32
lengths = (6, 4, 6, 4, 5, 5, 6)

for start in range(2, LIMIT, CHUNK):
    stop = min(start + CHUNK, LIMIT)
    seed = np.arange(start, stop, dtype=np.uint32)
    initial = seed * MUL + ADD
    initial |= np.uint32(1)
    state = after_constructors(initial)

    state, wanted = sums(state, lengths[0])
    state, got = sums(state, lengths[1])
    keep = got == wanted
    seed, state, wanted = seed[keep], state[keep], wanted[keep]

    for n in lengths[2:]:
        state, got = sums(state, n)
        keep = got == wanted
        seed, state, wanted = seed[keep], state[keep], wanted[keep]
        if not seed.size:
            break

    if seed.size:
        witness = int(seed[0])
        values = exact_values(witness)
        rows = []
        offset = 0
        for n in lengths:
            rows.append(values[offset:offset + n])
            offset += n
        print(f"seed={witness} constant={int(wanted[0])} rows={rows}")
        break

    if start and start % 100_000_000 == 2:
        print(f"searched_through={stop - 1}", flush=True)
else:
    raise SystemExit("no witness in 32-bit seed space")
