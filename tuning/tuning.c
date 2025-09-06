#include "tuning.h"
#include "data_parse.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_WEIGHTS (NUM_FEATURES + 1)

double evaluate_features(const Features *f, const double *weights) {
  double score = 0.0;
  for (int j = 0; j < NUM_FEATURES; ++j) {
    score += weights[j] * f->as_array[j];
  }
  return score;
}

static inline double clamp(double x, double lo, double hi) {
  if (x < lo)
    return lo;
  if (x > hi)
    return hi;
  return x;
}

static inline double sigmoid_scaled(double K, double q) {
  double x = K * q;
  x = clamp(x, -60.0, 60.0); /* avoid exp overflow */
  return 1.0 / (1.0 + exp(-x));
}

/* Compute mean squared error and analytic gradients:
   - loss = (1/N) * sum_i (s_i - R_i)^2
   - grad_params[j] = (1/N) * sum_i 2*(s_i - R_i) * (ds/dq) * f[i][j]
   - grad_K = (1/N) * sum_i 2*(s_i - R_i) * (ds/dq) * q_i / ??? (we derive
   below) where ds/dq = K * s * (1-s), and q_i = sum_j params[j]*f[i][j].
*/
double compute_loss_and_grads(const double *results, const Features *features,
                              size_t n, const double *params, double K,
                              double *grad_params, double *grad_K) {
  for (int j = 0; j < NUM_FEATURES; ++j)
    grad_params[j] = 0.0;
  if (grad_K)
    *grad_K = 0.0;

  double loss = 0.0;
  for (size_t i = 0; i < n; ++i) {
    /* compute q = params . f */
    double q = 0.0;
    for (int j = 0; j < NUM_FEATURES; ++j)
      q += params[j] * features[i].as_array[j];
    double s = sigmoid_scaled(K, q);
    double diff = s - results[i];
    loss += diff * diff;

    double ds_dq = K * s * (1.0 - s);
    double common = 2.0 * diff * ds_dq;
    for (int j = 0; j < NUM_FEATURES; ++j)
      grad_params[j] += common * features[i].as_array[j];

    if (grad_K) {
      /* derivative of s wrt K: ds/dK = (ds/dq) * q / K  *? careful:
         s = sigma(K*q), ds/dK = q * sigma'(K*q) = q * (s*(1-s))
         but sigma'(K*q) wrt q = K*s*(1-s). So:
         ds/dK = q * s*(1-s)
         Using the chain in MSE: dE/dK = (1/N) sum 2*(s-R)*(ds/dK) = (1/N) sum
         2*(s-R)*q*s*(1-s)
      */
      *grad_K += 2.0 * diff * q * (s * (1.0 - s));
    }
  }

  loss /= (double)n;
  for (int j = 0; j < NUM_FEATURES; ++j)
    grad_params[j] /= (double)n;
  if (grad_K)
    *grad_K /= (double)n;

  return loss;
}

double numeric_loss_params(const double *R, const Features *F, size_t N, int M,
                           const double *params, double K) {
  double *g = malloc(sizeof(double) * M);
  double gK;
  double L = compute_loss_and_grads(R, F, N, params, K, g, &gK);
  free(g);
  (void)gK;
  return L;
}

void texel_tuning() {
  Features *features;
  double *results;

  int *fixed_eval;
  int num_positions;

  if (!load_features_from_epd("book/random_positions.txt", &features, &results,
                              &fixed_eval, &num_positions)) {
    return;
  }

  printf("Loaded %d positions\n", num_positions);

  double rmin = 1e300, rmax = -1e300, rsum = 0;
  for (int i = 0; i < num_positions; ++i) {
    rmin = fmin(rmin, results[i]);
    rmax = fmax(rmax, results[i]);
    rsum += results[i];
  }
  printf("results: min=%g max=%g mean=%g\n", rmin, rmax, rsum / num_positions);

  int epochs = 2000;
  double lr = 0.02;
  double K = 0.85;
  double lrK = 0.005;
  double params[NUM_FEATURES] = {0};

  for (int j = 0; j < NUM_FEATURES; ++j) {
    params[j] = ((double)rand() / RAND_MAX - 0.5) * 0.01;
  }

  for (int e = 0; e < epochs; ++e) {
    double grad_params[NUM_FEATURES];
    double grad_K;

    double loss = compute_loss_and_grads(results, features, num_positions,
                                         params, K, grad_params, &grad_K);

    for (int j = 0; j < NUM_FEATURES; ++j) {
      params[j] -= lr * grad_params[j];
    }

    K -= lrK * grad_K;

    if (e % 10 == 0 || e == epochs - 1) {
      printf("Epoch %3d | loss = %.6f | K = %.6f\n", e, loss, K);
    }
  }

  for (int j = 0; j < NUM_FEATURES; ++j) {
    printf("Feature %2d: weight = %.6f\n", j, params[j]);
  }
}
