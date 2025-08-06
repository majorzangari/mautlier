#ifndef MAUTLIER_BITHELPERS_H
#define MAUTLIER_BITHELPERS_H

#define lsb_index(bb) (__builtin_ctzll(bb))
#define pop_lsb(bb) ((bb) &= (bb) - 1)

#define count_set_bits(x) __builtin_popcountll(x)

#endif
