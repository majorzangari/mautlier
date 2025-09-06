#ifndef MAUTLIER_DATA_PARSE_H
#define MAUTLIER_DATA_PARSE_H

#include "tuning.h"

#define MAX_POSITIONS 5000000

int load_features_from_epd(const char *filename, Features **features_out,
                           double **results_out, int **fixed_eval,
                           int *num_positions);

#endif
