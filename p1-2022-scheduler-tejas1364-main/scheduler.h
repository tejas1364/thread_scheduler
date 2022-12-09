#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>
#include <pthread.h>

#include "interface.h"

// Declare your own data structures and functions here...

void enqueuePV(int tid, int sem_id);
int dequeuePV();

void sortCPU();
void sortIO();
void enqueueCPU(float current_time, int tid, int remaining_time);
void dequeueCPU();
void specificDequeueCPU(int tid);

void enqueueIO(float current_time, int tid);
void dequeueIO();
int *createCondList(int total_threads);
int compareTid (const void * a, const void * b);
void sortByTid();
float getFirstCurrentTime();
int getCPUFirstTid();
int getIOFirstTid();
int getFirstRemainingTime();

pthread_cond_t* getCPUThreadCond(int tid);
pthread_cond_t* getIOThreadCond(int tid);
pthread_cond_t* getPVThreadCond(int tid);

int getRemainingTime(int tid);
int checkExistenceCPU(int tid);
int checkExistenceIO(int tid);
void decrementRemainingTime();

void printCPUList();
void printIOList();

void decrementSem(int sem_id);
void incrementSem(int sem_id);
int getSem(int sem_id);



#endif
