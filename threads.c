#include <stdio.h>
#include <pthread.h>

#define NTHREADS 40
#define MAXIMUM 1000000000

typedef struct {
	int id;
} thread_arg;

pthread_mutex_t mutex;
int value;

void *thread_function(void *vargp) {
	
	// convert the struct
	thread_arg *a = (thread_arg *) vargp;
	
	// increment 1 to value every time it can get the mutex
	do {
		pthread_mutex_lock(&mutex);
		//printf("Thread %d: before adding: %d\n", a->id+1, value);
		if (value < MAXIMUM) {
			value = value + 1;
		}
		//printf("Thread %d: after adding: %d\n", a->id+1, value);
		pthread_mutex_unlock(&mutex);
	} while (value < MAXIMUM);

	pthread_exit((void *)NULL);
}

int main(int argc, char **argv) {
	pthread_t tids[NTHREADS];
	thread_arg a[NTHREADS];
	int i;

	value = 0;
	
	// create mutex
	pthread_mutex_init(&mutex, NULL);

	// create the threads
	for (i=0; i < NTHREADS; i++) {
		a[i].id = i;
		pthread_create(&tids[i], NULL, thread_function, (void *)&(a[i]));
	}

	// wait for the threads to finish
	for (i = 0; i < NTHREADS; i++) {
		pthread_join(tids[i], NULL);
	}

	// Destroi o mutex
	pthread_mutex_destroy(&mutex);
	
	// imprime valor final
	printf("FINAL VALUE: %d\n", value);

	pthread_exit((void *)NULL);
}
