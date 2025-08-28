#ifndef MAUTLIER_MOVE_ORDERING_H
#define MAUTLIER_MOVE_ORDERING_H

#include "move.h"

int *score_moves(Board *pos, Move *moves, int scores[MAX_MOVES], int num_moves,
                 Move tt_move, int ply);

// returns the index of the best move in the given interval, after moving it to
// the front of the interval
// i.e. always returns start + 1
int pick_next_move(Move *moves, int *scores, int num_moves, int start, int end);

void add_killer(Move m, int depth);

void add_history(ToMove side, Move move, int depth);

#endif
