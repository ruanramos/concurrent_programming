#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <stdarg.h>
#include "timer.h"

typedef struct {
	int id;
	int *pointer;
	int max;
} thread_arg;

int array_index;
int count;
pthread_mutex_t mutex;

int PrintPrimes(int *arr, int max) {
	int t;
	for(t = 2; t < max; t++) {
		if(arr[t] == 0) { // is prime
			printf("%d ", t);
			count++;
		}
	}
	printf("\n");
	return count;
}

void *Crivo(void *args) {
	thread_arg * arg = (thread_arg *) args;
	int tid = arg->id;
	int *array = arg->pointer;
	int i, max;

	max = arg->max;

	// 0 = prime, 1 = not prime, cut
	do {
		// cut the not primes we find, multiples of the last prime
		pthread_mutex_lock(&mutex);
		for(i = 2*array_index; i < max; i += array_index) {
			array[i] = 1; // not prime, cut
		}
		pthread_mutex_unlock(&mutex);
		
		// lock mutex, change array_index global value, unlock mutex
		pthread_mutex_lock(&mutex);
		do { // check for next prime
			if (array_index < (int) sqrt(max)) {
				array_index++;
			}
		} while(array_index < (int) sqrt(max) && array[array_index] == 1);
		pthread_mutex_unlock(&mutex);
		
	} while (array_index < (int) sqrt(max));

	free(args);
	pthread_exit(NULL);

}

int main(int argc, char **argv) {
	pthread_t *tid_sistema; // threads identifier array

	int t; // for loops counter
	int nthreads, maxNumber; // number of threads to use and max number to check
	int *numbers;

	double start, end, delta;

	// validates and receive the entry values
	if(argc < 3) {
		printf("Use: %s <max number> <number of threads>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	maxNumber = atoi(argv[1]);
	nthreads = atoi(argv[2]);

	array_index = 2;

	// create mutex
	pthread_mutex_init(&mutex, NULL);

	// allocate memory for threads ids array
	tid_sistema = (pthread_t *) malloc(sizeof(pthread_t) * nthreads);
	if(tid_sistema==NULL) {
		printf("--ERRO: malloc()\n"); exit(-1);
	}
	
	// allocate memory for numbers array
	numbers = (int *) calloc(maxNumber+5,sizeof(int));
	if(numbers == NULL) {
		printf("--ERROR: malloc() failed\n"); exit(-1);
	}

	// -----------------------------------------------------------
	GET_TIME(start);

	// creating the threads
	for(t = 0; t < nthreads; t++)
	{
		thread_arg *args = (thread_arg *) malloc(sizeof(thread_arg));
		if(args == NULL) {
			printf("--ERROR: malloc() failed\n");
			exit(-1);
		}
		args->id = t;
		args->pointer = numbers;
		args->max = maxNumber;

		if(pthread_create(&tid_sistema[t], NULL, Crivo, (void *) args)) {
			printf("--ERRO: pthread_create()\n");
			exit(-1);
		}
	}
	// the main thread waits the other threads to finish
	for(t=0; t<nthreads; t++) {
		if (pthread_join(tid_sistema[t], NULL)) {
			printf("--ERRO: pthread_join()\n"); exit(-1);
		}
	}

	// Destroy mutex
	pthread_mutex_destroy(&mutex);

	GET_TIME(end);
	delta = end - start;
	//----------------------------------------------------------------

	//print result
	
	//printf("Number of primes: %d\n\n", PrintPrimes(numbers, maxNumber));
	printf("Time to calculate using %d threads: %f\n", nthreads, delta);

	free(tid_sistema);
	free(numbers);

	pthread_exit(NULL);
}