#ifndef MAUTLIER_TUNING_H
#define MAUTLIER_TUNING_H

#include "board.h"

#define MOBILITY_INDEX(piece) (piece - 2)

#define MOBILITY_KNIGHT 0
#define MOBILITY_BISHOP 1
#define MOBILITY_ROOK 2
#define MOBILITY_QUEEN 3

#define NUM_FEATURES 15

#define FEATURE_ISOLATED_PAWNS 0
#define FEATURE_DOUBLED_PAWNS 1
#define FEATURE_PASSED_PAWNS 2
#define FEATURE_BISHOP_PAIR 3
#define FEATURE_ROOK_OPEN_FILE 4
#define FEATURE_ROOK_SEMI_OPEN_FILE 5
#define FEATURE_CASTLED 6
#define FEATURE_KING_FRONT_PAWNS 7
#define FEATURE_KING_FRONT_SIDE_PAWNS 8
#define FEATURE_MOBILITY_KNIGHT 9
#define FEATURE_MOBILITY_BISHOP 10
#define FEATURE_MOBILITY_ROOK 11
#define FEATURE_MOBILITY_QUEEN 12
#define FEATURE_CENTER_CONTROL 13
#define FEATURE_TEMPO 14

typedef union {
  struct {
    int isolated_pawns;
    int doubled_pawns;
    int passed_pawns;
    int bishop_pair;
    int rook_open_file;
    int rook_semi_open_file;
    int castled;
    int king_front_pawns;
    int king_front_side_pawns;
    int mobility[4];
    int center_control;
    int tempo;
  };
  int as_array[NUM_FEATURES];
} Features;

char *features_to_string(const Features *f, char *buffer, size_t size);

double evaluate_features(const Features *f, const double *weights);

void texel_tuning();

#endif
