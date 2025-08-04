#define lsb_index(bb) (__builtin_ctzll(bb))
#define pop_lsb(bb) ((bb) &= (bb) - 1)
