#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#define NUM_THREADS 8
#define PARTS 800000000

typedef struct _args_integral {
    int part;
    int total;
    int thread;
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

void *pgenerate(void* arg) {
    args_integral *data = (args_integral*) arg;
    // printf("\nId: %d\n", data->thread);
    int val;
    for (int i = 0; i < data->part; i++) {
    	val = data->thread * data->part + i;
	//data->arr[i] = 4.0 / (1.0 + powl(val * 1.0 / data->total, 2)); 
    }
    pthread_exit(NULL);
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

int *pintegral(void *args){
    long double sum = 0.0;
    args_integral* data = (args_integral*) args;
    long double ipart = 1.0 / data->total;
    int val = data->thread * data->part;
    int part = data->part;
    long double var;
    for (int i = 0; i < part; i++) {
	var = (val + i) * 1.0 / data->total;	
	sum += 1.0 / (1.0 + (var * var)) * ipart;
    }
    psum[data->thread] = sum * 4;
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

int parallel(void) {
    // overheads
    pthread_t threads[NUM_THREADS];
    args_integral arg_array[NUM_THREADS]; 
    long taskids[NUM_THREADS];
    int rc;
    args_integral params;
    params.total = parts;
    params.part = parts / NUM_THREADS;
    for (int t = 0; t < NUM_THREADS; t++) {
        params.thread = taskids[t] = t;
	arg_array[t] = params;
	rc = pthread_create(&threads[t], NULL, pintegral, (void *) &arg_array[t]);	
	//printf("Thread %d\n", t);
    }
    // parallelable, but too annoying
    //for (int i = 0; i < parts; i++) {
    //    printf("%.11f ", yval[i]);
    //}
    long double final = 0.0;
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
	final += psum[i];
    }
    printf("Parallel Integral: %.11llf\n", final);
}

int main(void) {
    clock_t start, end;
    printf("%d", parts);
    start = clock();
    basic();
    end = clock();
    cpu_time = ((double) end - start) / CLOCKS_PER_SEC;
    printf("Base: %.5f s\n", cpu_time);
    start = clock();
    parallel();
    end = clock();
    cpu_time = ((double) end - start) / CLOCKS_PER_SEC;
    printf("Para: %.5f s\n", cpu_time);
}
