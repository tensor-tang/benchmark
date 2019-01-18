#include <assert.h>
#include <stdio.h>   // for printf
#include <stdlib.h>  // for malloc and free
#include <sys/time.h>
#include <random>
#include "mkl.h"

// or 4096
#define MEM_ALIGNMENT 64

template <typename T>
void random_vec(const int n, T* a, const T lower = static_cast<T>(-20.f),
               const T upper = static_cast<T>(20.f), unsigned int seed = 100) {
  std::mt19937 rng(seed);
  std::uniform_real_distribution<double> uniform_dist(0, 1);
  for (int i = 0; i < n; ++i) {
    a[i] = static_cast<T>(uniform_dist(rng) * (upper - lower) + lower);
  }
}

int main(int argc, char* argv[]) {
  void* A;
  void* B;
  void* C_col;
  void* C_row;
  float alpha = 1, beta = 0;

  int m = 1;
  int n = 1221;
  int k = 1221;


  int LOOP_COUNT = 10000;
  int BURNING_COUNT = 1000;

  if (argc == 1) {
    printf("Usage: %s m n k loop burning\n", argv[0]);
  }

  if (argc >= 2) m = atoi(argv[1]);
  if (argc >= 3) n = atoi(argv[2]);
  if (argc >= 4) k = atoi(argv[3]);
  if (argc >= 5) LOOP_COUNT = atoi(argv[4]);
  if (argc >= 6) BURNING_COUNT = atoi(argv[5]);


  // malloc data
  assert(posix_memalign(&A, MEM_ALIGNMENT, sizeof(float) * m * k) == 0);
  assert(posix_memalign(&B, MEM_ALIGNMENT, sizeof(float) * k * n) == 0);
  assert(posix_memalign(&C_row, MEM_ALIGNMENT, sizeof(float) * m * n) == 0);
  assert(posix_memalign(&C_col, MEM_ALIGNMENT, sizeof(float) * m * n) == 0);

	random_vec<float>(m*k, (float*)(A), -2.f, 2.f);
	random_vec<float>(k*n, (float*)(B), -2.f, 2.f);

  auto GetCurrentUs = []() ->double {
    struct timeval time;
    gettimeofday(&time, NULL);
    return 1e+6 * time.tv_sec + time.tv_usec;
  };

  bool transA = false;
  bool transB = false;
  int lda = transA == false ? k : m;
  int ldb = transB == false ? n : k;
  int ldc = n;

#define row_sgemm                                          \
  cblas_sgemm(CblasRowMajor,                               \
              transA == false ? CblasNoTrans : CblasTrans, \
              transB == false ? CblasNoTrans : CblasTrans, \
              m,                                           \
              n,                                           \
              k,                                           \
              alpha,                                       \
              (float*)A,                                   \
              lda,                                         \
              (float*)B,                                   \
              ldb,                                         \
              beta,                                        \
              (float*)C_row,                               \
              ldc)

#define col_sgemm                                          \
  cblas_sgemm(CblasColMajor,                               \
              transA == false ? CblasNoTrans : CblasTrans, \
              transB == false ? CblasNoTrans : CblasTrans, \
              n,                                           \
              m,                                           \
              k,                                           \
              alpha,                                       \
              (float*)B,                                   \
              ldb,                                         \
              (float*)A,                                   \
              lda,                                         \
              beta,                                        \
              (float*)C_col,                               \
              ldc)

#define for_times(num, line) \
  for (int i = 0; i < num; ++i) { \
    line; \
  }

  for_times(BURNING_COUNT, row_sgemm);
  auto t1 = GetCurrentUs();
  for_times(LOOP_COUNT, row_sgemm);
  auto t2 = GetCurrentUs();

  for_times(BURNING_COUNT, col_sgemm);
  auto t3 = GetCurrentUs();
  for_times(LOOP_COUNT, col_sgemm);
  auto t4 = GetCurrentUs();

	printf("Burning times: %d, repeat times: %d \n", BURNING_COUNT, LOOP_COUNT);
  printf("m=%d, n=%d, k=%d row_sgemm_us=%.3f, col_sgemm_us=%.3f\n",
    m, n, k, (t2-t1)/LOOP_COUNT, (t4-t3)/LOOP_COUNT);

  float* c1 = (float*)C_row;
  float* c2 = (float*)C_col;
  for (int i=0; i < m*n; ++i) {
    auto diff = c1[i] - c2[i];
    auto absdiff = diff < 0 ? -diff : diff;
    if (absdiff > 1e-5) {
      printf("id %d has diff: %f \n", i, diff);
    }
  }
    
  free(A);
  free(B);
  free(C_row);
  free(C_col);
  return 0;
}
