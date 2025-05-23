import random as rand
import numpy as np

RANK_1: np.uint64 = np.uint64(0x00000000000000FF)
FILE_A: np.uint64 = np.uint64(0x8080808080808080)
RANK_8: np.uint64 = np.uint64(0xFF00000000000000)
FILE_H: np.uint64 = np.uint64(0x0101010101010101)


def get_blocker_mask(square: int) -> np.uint64:
    mask = np.uint64(0)
    rank, file = divmod(square, 8)

    directions = [(-1, -1), (-1, 1), (1, -1), (1, 1)]

    for dr, df in directions:
        r, f = rank + dr, file + df
        while 0 < r < 7 and 0 < f < 7:
            sq = np.uint64(r * 8 + f)
            mask |= np.uint64(1) << sq
            r += dr
            f += df

    mask &= ~(np.uint64(1) << np.uint64(square))
    mask &= ~(RANK_1 | RANK_8 | FILE_A | FILE_H)

    return mask


def bitset_subsets(bitset: np.uint64) -> list[np.uint64]:
    subsets = []
    subset = bitset
    while True:
        subsets.append(subset)
        if subset == 0:
            break
        subset = (subset - 1) & bitset
    return subsets


def get_bishop_attacks(blockers: np.uint64, square: int) -> np.uint64:
    attacks = np.uint64(0)
    rank, file = divmod(square, 8)

    directions = [(1, 1), (1, -1), (-1, 1), (-1, -1)]
    for dr, df in directions:
        for i in range(1, 8):
            r = rank + (i * dr)
            f = file + (i * df)
            if r > 7 or f > 7 or r < 0 or f < 0:
                break
            sq = r * 8 + f
            attacks |= np.uint64(1) << np.uint64(sq)
            if blockers & (np.uint64(1) << np.uint64(sq)):
                break

    return attacks


def get_all_bishop_attacks(square: int) -> dict[np.uint64, np.uint64]:
    blocker_mask = get_blocker_mask(square)
    subsets = bitset_subsets(blocker_mask)
    return {blocker: get_bishop_attacks(blocker, square) for blocker in subsets}


def get_index_bits(square_index: int) -> int:
    index_bits = 5
    if (
        square_index == 0
        or square_index == 7
        or square_index == 56
        or square_index == 63
    ):
        index_bits = 6

    rank, file = divmod(square_index, 8)

    if rank >= 2 and rank <= 5 and file >= 2 and file <= 5:
        index_bits = 7
    if rank >= 3 and rank <= 4 and file >= 3 and file <= 4:
        index_bits = 9
    return index_bits


def find_magic(square: int) -> np.uint64:
    attack_map = get_all_bishop_attacks(square)
    index_bits = get_index_bits(square)
    blocker_mask = get_blocker_mask(square)
    map = {}

    while True:
        fail = False
        magic = (
            np.uint64(rand.getrandbits(64))
            & np.uint64(rand.getrandbits(64))
            & np.uint64(rand.getrandbits(64))
        )
        map = {}
        for blocker in bitset_subsets(blocker_mask):
            index = (blocker * magic) >> (64 - index_bits)
            if index not in map:
                map[index] = attack_map[blocker]
            elif map[index] != attack_map[blocker]:
                fail = True
                break
        if not fail:
            return magic


if __name__ == "__main__":
    for i in range(64):
        magic = find_magic(i)
        print(hex(magic) + ",")
