/*######################################_Pier_Luigi_Manfrini_#################################
 *
 *	This Program searches for all prime numbers below a certain number N.
 *	N is provided by the user through CLI as well as the number of desired
 *	workers (threads) T.
 *
 *	Usage:
 *		<./solution09> <N> <T>
 *
 *	Notes:
 *		-DEBUG macro flag available in order to check ranges assigned to workers
 *
 *
 *	Compile:
 *		gcc solution09.c -o solution09 -Wall -Werror -pthread -lm -fsanitize=leak
 *	
 * ###########################################################################################
 */

#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define RED "\033[1;31m"
#define RESET "\033[0m"

#define DEBUG 0 // 0 or 1

size_t size = 0;
int *prime_numbers = NULL;
size_t next_in = 0;

typedef struct {
  int lower_bound;
  int upper_bound;
} thr_arg;

pthread_mutex_t mutex;
pthread_cond_t cond;

void quicksort(int first, int last) {
  int i, j, pivot, temp;
  if (first < last) {
    pivot = first;
    i = first;
    j = last;

    while (i < j) {
      while (prime_numbers[i] <= prime_numbers[pivot] && i < last)
        i++;
      while (prime_numbers[j] > prime_numbers[pivot])
        j--;
      if (i < j) {
        temp = prime_numbers[i];
        prime_numbers[i] = prime_numbers[j];
        prime_numbers[j] = temp;
      }
    }

    temp = prime_numbers[pivot];
    prime_numbers[pivot] = prime_numbers[j];
    prime_numbers[j] = temp;
    quicksort(first, j - 1);
    quicksort(j + 1, last);
  }
}

void *find_prime(void *arg) {
  thr_arg *input = arg;
  for (int i = input->lower_bound; i <= input->upper_bound; i++) {
    bool not_prime = false;
    if (i == 1)
      not_prime = true;
    for (int j = 2; j <= floor(i / 2); j++) {
      if (i % j == 0) {
        not_prime = true;
        break;
      }
    }
    if (!not_prime) {
      /* Mutex LOCK */
      pthread_mutex_lock(&mutex);
      /* reserve heap space the first time */
      if (prime_numbers == NULL) {
        size = 100;
        prime_numbers = (int *)malloc(size * sizeof(int));
      }
      /* Realloc if running out of space (75% filled up) */
      else if (next_in == size - floor(size * 75 / 100)) {
        size = (size_t)size * 1.5;
        prime_numbers = (int *)realloc(prime_numbers, size);
      }
      prime_numbers[next_in] = i;
      next_in++;
      pthread_mutex_unlock(&mutex);
      /* Mutex UNLOCK */
    }
  }

  pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
  if (argc != 3) {
    dprintf(2, "Wrong Usage: <./solution09> <N> <T>\n");
    exit(EXIT_FAILURE);
  }
  int N, T;
  N = atoi(argv[1]);
  T = atoi(argv[2]);
  if (N <= 0 || T <= 0) {
    dprintf(2, "N and T must be positive integers!\n");
    exit(EXIT_FAILURE);
  } else if (N == 1) {
    dprintf(2, "Nobody knows...is 1 prime?\n");
    exit(EXIT_FAILURE);
  } else if (T > N / 2) {
    dprintf(2, "The number of threads (T) has to be lower than N/2!\n");
    exit(EXIT_SUCCESS);
  }
  clock_t begin = clock();
  /* Mutex Initialization */
  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&cond, NULL);

  /* Threads Initialization */
  pthread_t *threads = (pthread_t *)malloc(T * sizeof(pthread_t));
  pthread_attr_t attribute;
  pthread_attr_init(&attribute);
  pthread_attr_setdetachstate(&attribute, PTHREAD_CREATE_JOINABLE);

  thr_arg *args = (thr_arg *)malloc(T * sizeof(thr_arg));

  /* Threads Creation */
  for (int i = 0; i < T; i++) {
    args[i].lower_bound = (floor(N / T) * i) + 1;
    if (DEBUG)
      dprintf(1, "thr %d LOWER:%d\t", i, args[i].lower_bound);
    if (i == T - 1)
      args[i].upper_bound = N;
    else
      args[i].upper_bound = args[i].lower_bound + (floor(N / T) - 1);
    if (DEBUG)
      dprintf(1, "UPPER:%d\n", args[i].upper_bound);
    pthread_create(&threads[i], &attribute, find_prime, (void *)&args[i]);
  }

  /* Join terminated threads */
  for (int i = 0; i < T; i++) {
    pthread_join(threads[i], NULL);
  }

  /* Sort prime numbers' vector */
  quicksort(0, next_in - 1);
  clock_t end = clock();
  /* Print all prime numbers found */
  dprintf(1, RED "Prime Numbers smaller than %d:" RESET "\n", N);
  for (int i = 0; i < next_in; i++) {
    dprintf(1, "%d%s", prime_numbers[i], ((i + 1) % 15) ? "\t" : "\n");
  }
  dprintf(1, "\n(took %7.6f seconds)\n",
          (double)(end - begin) / CLOCKS_PER_SEC);
  free(threads);
  free(args);
  free(prime_numbers);
  return 0;
}
