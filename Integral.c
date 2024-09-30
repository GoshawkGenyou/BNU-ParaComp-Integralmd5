#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define NUM_THREADS 4
#define PARTS 484000

typedef struct _args_integral {
    unsigned long long part_start;
    unsigned long long part_end;
    long double width;
    int thread;
    struct {
    	long double value;
    	pthread_mutex_t mutex;
    } *result;
} args_integral;

long double psum[NUM_THREADS] = { 0 };
int const parts = PARTS;
double cpu_time;
int myintegral(double *arr, int part);

// parallelable
int generate_func(int part) {
    double ipart = 1.0 / part;
    long double sum = 0.0;
    long double var;
    for (int i = 0; i < part; i++){
	var = (i * 1.0 / part);
	sum += 4.0 / (1.0 + (var * var)) * ipart;
    }
    printf("\n%.11llf\n", sum);
}

int myintegral(double *arr, int part) {
    double sum = 0.0;
    double ipart = 1.0/part;
    for (int i = 0; i < part; i++) {
        sum += ipart * arr[i];
    }
    printf("\n%.11lf\n", sum);
    return 0;
}

void *pintegral(void *args){
    long double sum = 0.0;
    args_integral* data = (args_integral*) args;
    long double var;
    for (unsigned long long i = data->part_start; i < data->part_end; i++) {
		var = (i + 0.5) * data->width; // midpoint
        sum += 4.0 / (1.0 + var * var);
    }
    pthread_mutex_lock(&data->result->mutex);
    data->result->value += sum * data->width; // update the result
    pthread_mutex_unlock(&data->result->mutex);

    pthread_exit(NULL);
};

int basic(void) {
    generate_func(parts);
    /**for (int i = 0; i < parts; i++) {
        printf("%.11f ", yval[i]);
    }
    */
    return 0;
}

void partitioner(unsigned long long *parts) {
	unsigned long long chunk = (PARTS + NUM_THREADS - 1) / NUM_THREADS; // Ceiling division
	for (unsigned long long i = 0; i < NUM_THREADS + 1; i++) {
		parts[i] = chunk * i;
	}
	parts[NUM_THREADS] = PARTS;
}

int parallel(void) {
    // overheads
    pthread_t threads[NUM_THREADS];
    args_integral arg_array[NUM_THREADS]; 
    long taskids[NUM_THREADS];
    args_integral params;
    unsigned long long chunks[NUM_THREADS + 1];
    long double width = 1.0/ PARTS;
	memset(chunks, 0, 5);
	partitioner(&chunks);
    struct {
        long double value;
        pthread_mutex_t mutex;
    } result = {0.0, PTHREAD_MUTEX_INITIALIZER};
    params.result = &result;
    
    for (int t = 0; t < NUM_THREADS; t++) {
        params.part_start = chunks[t];
    	params.part_end = chunks[t + 1]; // no risk of segfault as chunk always NUM + 1
        params.thread = taskids[t] = t;
        params.width = width;
		arg_array[t] = params;
		pthread_create(&threads[t], NULL, pintegral, (void *) &arg_array[t]);	
	//printf("Thread %d\n", t);
    }
    // parallelable, but too annoying
    //for (int i = 0; i < parts; i++) {
    //    printf("%.11f ", yval[i]);
    //}
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("%.11llf\n", result.value);
}

int main(void) {
	/* verify
	int chunks[NUM_THREADS + 1];
	memset(chunks, 0, 5);
	partitioner(&chunks);
	for (int i = 0; i < NUM_THREADS + 1; i++) {
		printf("%d ", chunks[i]);
	}
	printf("\n");
	*/
	
    /*
    clock_t start, end;
    printf("%d", parts);
    start = clock();
    basic();
    end = clock();
    cpu_time = ((double) end - start) / CLOCKS_PER_SEC;
    printf("Base: %.5f s\n", cpu_time);
    start = clock();
    */
    
    parallel();
    /*
    end = clock();
    cpu_time = ((double) end - start) / CLOCKS_PER_SEC;
    printf("Para: %.5f s\n", cpu_time);
    */
}
