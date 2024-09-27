#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdatomic.h>
#include "src/md5.c"
#include <sys/time.h>
#define LEN 7
#define PLAINNUM 36
#define PLAIN "hello\0"
#define THREADS 9

MD5_CTX base;

typedef struct {
    int thread_id;
    unsigned long long start;
    unsigned long long end;
    bool execution;
} pmd5_arg;

int const plainnum = PLAINNUM;
char plaindef[PLAINNUM];
unsigned int len;
unsigned long long total_comb;

void randomizer(char* pl) {
    srand(time(0));
    for(int i = 0; i < LEN; i++) {
        pl[i] = plaindef[rand() % PLAINNUM];
    }
}


int init(){
    int i;
    for (i = 0; i < 26; i++) {
    	plaindef[i] = 97 + i;
	//plaindef[26 + i] = 65 + i;
    }
    int j = i;
    for (; i < j + 10; i++) {
	    plaindef[i] = 48 + i - j;
    }
    char plain[LEN + 1];
    memset(plain, 0, LEN);
    randomizer(plain);
    plain[LEN] = '\0';
    printf("Original: %s\n", plain);
    len = LEN;
    total_comb = pow(plainnum, len);
    MD5Init(&base);
    MD5Update(&base, plain, len);
    MD5Final(&base);
    for (int i = 0; i < 16; i++) {
	printf("%02x", base.digest[i]);
    }
    printf("\n");

    return 0;	
    
}

int cmpmd5(char *message1, char *message2) {
    for (int i = 0; i < 16; i++) {
	if (message1[i] != message2[i]) return 0;
    }
    return 1;
}

int baseattempt(){
    char plain[len + 1];
    for (int i = 0; i < len; i++) {
    	plain[i] = plaindef[0];
    }
    plain[len] = '\0';
    MD5_CTX md5Guess;
    int plainidx = len - 1;
    int iter = 0;
    for(int m = 0; m < plainnum; m++){
    for(int k = 0; k < plainnum; k++) {
    for(int j = 0; j < plainnum; j++) {
    for(int i = 0; i < plainnum; i++) {
        MD5Init(&md5Guess);
        MD5Update(&md5Guess, plain, len);
        MD5Final(&md5Guess);
	if (!cmpmd5(md5Guess.digest, base.digest)) {
	    plain[plainidx] = plaindef[i];
	    continue;
	} else {
    // found
    printf("%s\n", plain);
    for (int i = 0; i < 16; i++) {
	printf("%02x", md5Guess.digest[i]);
    }
    printf("\n");
	    return 0;
	}
    }
    plain[plainidx - 1] = plaindef[j];
    }
    plain[plainidx - 2] = plaindef[k];
    }
    plain[plainidx - 3] = plaindef[m];
    }
    printf("%s\n", plain);
    for (int i = 0; i < 16; i++) {
	printf("%02x", md5Guess.digest[i]);
    }
    printf("\n");
}

int char_to_index(char c){
    for(int i = 0; i < plainnum; i++) {
	    if (plaindef[i] == c) {
	        return i;
	    }
    }
    return -1;
}


// Function to get the combination index for the given string
unsigned long long get_combination_index(const char* str) {
    unsigned long long index = 0;

    for (int i = 0; i < len; i++) {
        int pos = char_to_index(str[i]);
        if (pos < 0) {
            printf("Invalid character: %c\n", str[i]);
            return -1; // Error handling for invalid characters
        }
        // Calculate base raised to the power of the current index
        unsigned long long power = 1;
        for (int j = 0; j < (len - 1 - i); j++) {
            power *= plainnum; // Use plainnum as the base
        }
        index += pos * power;
    }

    return index;
}

// Function to convert a combination index back to a string
void get_string_from_index(unsigned long long index, char* result) {
    // Ensure result is null-terminated
    memset(result, 0, len + 1);
    
    for (int i = 0; i < len; i++) {
        // Calculate position for the current character
        unsigned long long pos = index % plainnum;
        result[len - 1 - i] = plaindef[pos]; // Set character in reverse order
        index /= plainnum; // Move to the next digit
    }
}

int baseattempt2(){
    MD5_CTX md5Guess;
    char plain[len + 1];
    memset(plain, plaindef[0], len);
    plain[len] = '\0'; // Null-terminate the string

    for (unsigned long long i = 0; i < total_comb; i++) {
        get_string_from_index(i, &plain);
        MD5Init(&md5Guess);
        MD5Update(&md5Guess, plain, len);
        MD5Final(&md5Guess);
	if (cmpmd5(md5Guess.digest, base.digest)) {
    	    // found
    	    printf("Normal found: %s\nHash Validation: ", plain);
    	    for (int i = 0; i < 16; i++) {
		printf("%02x", md5Guess.digest[i]);
    	    }
    	printf("\n");
	    return 1;
	}
    }
    return 0;
}


atomic_bool found = false;

void *md5_thread(void* args){
    pmd5_arg* arg = (pmd5_arg*)args;
    if (atomic_load(&found)) {
        arg->execution = 0;
	    return NULL;
	}
	arg->execution = 1;
    char plain[len + 1];
    MD5_CTX md5Guess;
    printf("Thread %d processing characters: %lld to %lld\n", arg->thread_id, arg->start, arg->end);
    for (unsigned long long i = arg->start; i < arg->end; i++) {
	    // another thread / current thread found it.	
	    if (atomic_load(&found)) {
	        return NULL;
	    }
	    // Build the string from the index
	    get_string_from_index(i, &plain);
        MD5Init(&md5Guess);
        MD5Update(&md5Guess, plain, len);
        MD5Final(&md5Guess);
        
	if (cmpmd5(md5Guess.digest, base.digest)) {
	    // found
	    atomic_store(&found, true);
	    printf("Thread %d found: %s\n", arg->thread_id, plain);
	    /**
	    for (int i = 0; i < 16; i++) {
    		printf("%02x", md5Guess.digest[i]);
	    }
	    */
	    return NULL;
	}
    }
    printf("Thread %d exited at %s\n", arg->thread_id, plain);
    return NULL;
}

int partition(unsigned long long *arr){
    char s[len + 1];
    for (int i = 0; i < len; i++) {
	s[i] = plaindef[0];
    }
    s[len] = '\0';
    for (int i = 0; i < plainnum; i++) {
	s[0] = plaindef[i];
	arr[i] = get_combination_index(s);
    }

}

int thread_manager(){
    double thread_time = 0.0;
    double aggregate_time = 0.0;
    int tactive = 0;
    pthread_t threads[THREADS];
    pmd5_arg args[THREADS];
    unsigned long long chunk_size = pow(plainnum, len - 1)  / plainnum;
    unsigned long long chunk_part[plainnum];
    partition(&chunk_part);
    int active = 0;
    for (int j = 0; j < plainnum / THREADS; j++) {
        int k = j * THREADS;
    	for (int i = 0; i < THREADS; i++) {
	        args[i].thread_id = i;
	        args[i].start = chunk_part[k + i];
	        args[i].end = (k + i + 1 < plainnum) ? chunk_part[k + i + 1] : total_comb;
	        pthread_create(&threads[i], NULL, md5_thread, &args[i]);
    	}
    	for (int i = 0; i < THREADS; i++) {
	        pthread_join(threads[i], NULL);
	        if(args[i].execution) active++;
        }
    	active = 0;
        tactive += active;        
    }
    //double average_time = 0.0;
    //if (tactive > 0) average_time = aggregate_time / tactive; // averaged clocks
    printf("Threads Run: %d\n", tactive);
    return 0;
}


int main() {
    struct timeval start, end;
    double elapsed_time;
    init();
    /**
    printf("%lld\n", get_combination_index("aaaaa"));
    printf("%lld\n", get_combination_index("aaaab"));
    printf("%lld\n", get_combination_index("aaaac"));
    printf("%lld\n", get_combination_index("daaaa"));
    printf("%lld\n", get_combination_index("00000"));
    printf("%lld\n", get_combination_index("99999"));
    char testplain[len + 1];
    get_string_from_index(get_combination_index("99999"), &testplain);
    printf("%s\n", testplain);
    */
    
    //baseattempt();

    gettimeofday(&start, NULL); // Start timing
    //baseattempt2();
    gettimeofday(&end, NULL); // End timing
    elapsed_time = (end.tv_sec - start.tv_sec) + 
                          (end.tv_usec - start.tv_usec) / 1e6;
    printf("Normal executed in %.6fs\n", elapsed_time);
    
    gettimeofday(&start, NULL); // Start timing
    thread_manager();
    gettimeofday(&end, NULL); // End timing
    elapsed_time = (end.tv_sec - start.tv_sec) + 
                          (end.tv_usec - start.tv_usec) / 1e6;
    printf("Parallel executed in %.6fs\n", elapsed_time);
}
