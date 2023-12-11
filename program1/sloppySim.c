#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>

#define DEFAULT_N_THREADS  2
int N_Threads = DEFAULT_N_THREADS;
int Sloppiness = 10;
int work_time = 10;
int work_iterations = 100;
bool CPU_BOUND = false;
bool Do_Logging = false;
pthread_t* threads;

typedef struct __counter_t
{
    int globalCount;
    sem_t glock;
    int localCount[DEFAULT_N_THREADS];
    sem_t llock[DEFAULT_N_THREADS];
    int Sloppiness;
} counter_t;

// Initialize counter, and sloppiness 
void init(counter_t *c, int s) {
    c->Sloppiness = s;
    c->globalCount = 0;
    sem_init(&c->glock, 0, 1);
    for(int i=0;i<DEFAULT_N_THREADS;i++) {
        c->localCount[i] = 0;
        sem_init(&c->llock[i], 0, 1);
    }
}

int getThreadID(pthread_t threadID) {
    return (int)threadID;
}

// Updates the global count once local count has grown by 'threshold'
void update(counter_t *c, pthread_t threadID, int amt) {
    int cpu = getThreadID(threadID) % N_Threads;
    sem_wait(&c->llock[cpu]);
    c->localCount[cpu] += amt;
    if(c->localCount[cpu] == c->Sloppiness) {
        sem_wait(&c->glock);
        c->globalCount += c->localCount[cpu];
        sem_post(&c->glock);
        c->localCount[cpu] = 0;
    }
    sem_post(&c->llock[cpu]);
}

// Returns the approximate global count
int get(counter_t *c) {
    sem_wait(&c->glock);
    int val = c->globalCount;
    sem_post(&c->glock);
    return val;
}

// Simulates the work done by each thread
void* work(void* arg) {
    counter_t *counter = (counter_t *)arg;
    int threadID = pthread_self();
    int amt = 1;

    for(int i=0;i<work_iterations;i++) {
        // measure time
        if(CPU_BOUND) {
            long increments = work_time*1e6;
            for(int j=0;j<increments;++j);
            // // CPU is Busy
        } else {
            // CPU is waiting (I/O work)
            usleep(work_time * 1000);
        }
        update(counter, threadID, amt);
    }

    //pthread_exit(NULL);
    return NULL;
}

int main(int argc, char *argv[]) {
    if(argc > 1) N_Threads = atoi(argv[1]);
    if(argc > 2) Sloppiness = atoi(argv[2]);
    if(argc > 3) work_time = atoi(argv[3]);
    if(argc > 4) work_iterations = atoi(argv[4]);
    // Convert strings from command line to booleans
    if(argc > 5) CPU_BOUND = (argv[5][0] == 't' || argv[5][0] == 'T') ? true : false;
    if(argc > 6) Do_Logging = (argv[6][0] == 't' || argv[6][0] == 'T') ? true : false;

    threads = (pthread_t*)malloc(N_Threads * sizeof(pthread_t));
    counter_t c;
    init(&c, Sloppiness);

    for(int i=0;i<N_Threads;i++) {
        pthread_create(&threads[i], NULL, work, (void *)&c);
    }

    for(int i=0;i<N_Threads;i++) {
        pthread_join(threads[i], NULL);
    }

    // Print current settings if logging is enabled
    printf("with simulation parameters:\n\tN_Threads=%d\n \tSloppiness=%d\n \tWorkTime=%d\n \tWorkIterations=%d\n \tCPU_Bound=%s\n \tDoLogging=%s\n",
            N_Threads, Sloppiness, work_time, work_iterations, CPU_BOUND ? "true" : "false", Do_Logging ? "true" : "false");

    printf("Final Global Count: %d\n", get(&c));

    //sem_destroy(&c);

    free(threads);

    return 0;
}