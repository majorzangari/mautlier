#include "data_parse.h"
#include "board.h"
#include "eval.h"
#include "fen.h"
#include "tuning.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 256

double parse_result(const char *res) {
  if (strcmp(res, "1-0") == 0)
    return 1.0;
  if (strcmp(res, "0-1") == 0)
    return 0.0;
  if (strcmp(res, "1/2-1/2") == 0)
    return 0.5;
  return 0.5; // default to draw if unrecognized
}

int load_features_from_epd(const char *filename, Features **features_out,
                           double **results_out, int **fixed_eval,
                           int *num_positions) {
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    perror("Failed to open EPD file");
    return 0;
  }

  Features *features = malloc(sizeof(Features) * MAX_POSITIONS);
  double *results = malloc(sizeof(double) * MAX_POSITIONS);
  int *fixed = malloc(sizeof(int) * MAX_POSITIONS);
  char line[MAX_LINE];
  int count = 0;

  while (fgets(line, sizeof(line), fp)) {
    if (count >= MAX_POSITIONS)
      break;

    line[strcspn(line, "\r\n")] = 0;

    char *res_ptr = strchr(line, ';');
    if (!res_ptr)
      continue;

    *res_ptr = '\0';
    char *fen = line;

    char *anno = res_ptr + 1;
    while (*anno == ' ')
      anno++;

    char *tag = strstr(anno, "result \"");
    if (!tag)
      continue;

    tag += 8; // skip past 'result "'
    char *end_quote = strchr(tag, '"');
    if (!end_quote)
      continue;

    *end_quote = '\0';
    const char *res_str = tag;

    double result = parse_result(res_str);

    Board *board = fen_to_board(fen);
    if (!board)
      continue;

    Features f = extract_features(board);
    int fixed_eval = fixed_evaluation(board);
    features[count] = f;
    fixed[count] = fixed_eval;
    results[count] = result;
    count++;

    // if (count % 1000 == 0) {
    //   printf("Loaded %d positions\n", count);
    //   printf("Last FEN: %s\n", fen);
    //   printf("Last result: %s -> %.1f\n", res_str, result);
    //   char buf[256];
    //   printf("Last features: %s", features_to_string(&f, buf, sizeof(buf)));
    //   char *board_str = board_to_string(board);
    //   printf("Board:\n%s\n", board_str);
    // }

    free(board);
  }

  fclose(fp);
  *features_out = features;
  *results_out = results;
  *fixed_eval = fixed;
  *num_positions = count;
  return 1;
}
