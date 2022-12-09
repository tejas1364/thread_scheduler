#include "scheduler.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Scheduler implementation
// Implement all other functions here...

struct node {
    float current_time;
    int tid;
    int remaining_time;
    pthread_cond_t thread_cond;
    struct node *next;
};

typedef struct pthread_cond_data {
    int tid;
    pthread_cond_t thread_cond;
    int semID;
} new_condition;

typedef struct semaphoreData {
    int semID;
    int semaphore;
} semaphore;


// For CPU_ME
int count = 0;
int countIO = 0;
int thread_count = 0;
int countPV = 0;

struct node *headCPU = NULL, *tailCPU = NULL, *temp = NULL;
static new_condition *condListCPU, *condListIO, *condListPV;
static semaphore *semList;

// For IO_ME
struct node *headIO = NULL, *tailIO = NULL;// *ioTemp = NULL;
//pthread_cond_t *temp_cond;


void specificDequeueCPU(int tid){
    struct node *temp1 = headCPU, *prev;
    //condListCPU[tid].thread_cond = NULL;
    if (temp1 != NULL && temp1->tid == tid) {
        headCPU = temp1->next; 
        free(temp1);
	return;
    }
    while (temp1 != NULL && temp1->tid != tid) {
        prev = temp1; 
        temp1 = temp1->next;
    }

    if (temp1 == NULL){
        return;
    }
 
    prev->next = temp1->next;
    free(temp1);
    
}


void sortCPU() {

   int i, j, k, tempTID, tempRemainingTime;
   float tempCurrentTime;
   //pthread_cond_t tempThreadCondition;
   struct node *current;
   struct node *next;
	
   //int size = length();
   k = count;
	
   for ( i = 0 ; i < count - 1 ; i++, k-- ) {
      current = headCPU;
      next = headCPU->next;
		
      for ( j = 1 ; j < k ; j++ ) {   

        if ( current->current_time > next->current_time ) {
            tempCurrentTime = current->current_time;
            tempTID = current->tid;
            tempRemainingTime = current->remaining_time;
            
            //tempThreadCondition = current->thread_cond;
            
            current->current_time = next->current_time;
            next->current_time = tempCurrentTime;
            
            current->tid = next->tid;
            next->tid = tempTID;
            
            current->remaining_time = next->remaining_time;
            next->remaining_time = tempRemainingTime;
            
        }
        
        else if(current->current_time == next->current_time){
            if(current->tid > next->tid){
                tempCurrentTime = current->current_time;
                tempTID = current->tid;
                tempRemainingTime = current->remaining_time;
                
                //tempThreadCondition = current->thread_cond;
            
                current->current_time = next->current_time;
                next->current_time = tempCurrentTime;
            
                current->tid = next->tid;
                next->tid = tempTID;
            
                current->remaining_time = next->remaining_time;
                next->remaining_time = tempRemainingTime;

            }
        } 
        
        current = current->next;
        next = next->next;
      }
   }   
}

int compareTid (const void * a, const void * b) {
    if((((new_condition*)a)->tid) < ((new_condition*)b)->tid) return -1;
    
    if((((new_condition*)a)->tid) == ((new_condition*)b)->tid){
        return 0;
    }
    return 1;
}

void sortByTid(){
    qsort(condListPV, countPV, sizeof(new_condition), compareTid);
}

/*
int getTid(){
    return queue[0].tid;
} 

pthread_cond_t getCondition(){
    return queue[0].thread_cond;
} */


void enqueuePV(int tid, int sem_id) {
    condListPV[countPV].tid = tid;
    condListPV[countPV].semID = sem_id;
    pthread_cond_init(&condListPV[countPV].thread_cond, NULL);
    countPV++;
    sortByTid(); 
}

int dequeuePV() {
    new_condition head = condListPV[0];
    
    for (int i = 0; i < countPV - 1; i++) {
        condListPV[i] = condListPV[i + 1];
    }
    countPV--;
    return 0;
}

pthread_cond_t* getPVThreadCond(int tid){
    for(int i = 0; i < thread_count; i++){
        if(condListPV[i].tid == tid){
            return &condListPV[i].thread_cond;
        }
    }
    //return &condListPV[tid].thread_cond;
    return NULL;
}

// /// // // // // / 

void sortIO() {

   int i, j, k, tempTID;
   float tempCurrentTime;
   //pthread_cond_t tempThreadCondition;
   struct node *current;
   struct node *next;
	
   //int size = length();
   k = countIO;
	
   for ( i = 0 ; i < countIO - 1 ; i++, k-- ) {
      current = headIO;
      next = headIO->next;
		
      for ( j = 1 ; j < k ; j++ ) {   

        if ( current->current_time > next->current_time ) {
            tempCurrentTime = current->current_time;
            tempTID = current->tid;
            //tempRemainingTime = current->remaining_time;
            
            //tempThreadCondition = current->thread_cond;
            
            current->current_time = next->current_time;
            next->current_time = tempCurrentTime;
            
            current->tid = next->tid;
            next->tid = tempTID;
            
            //current->remaining_time = next->remaining_time;
            //next->remaining_time = tempRemainingTime;
            
        }
        
        else if(current->current_time == next->current_time){
            if(current->tid > next->tid){
                tempCurrentTime = current->current_time;
                tempTID = current->tid;
                //tempRemainingTime = current->remaining_time;
                
                //tempThreadCondition = current->thread_cond;
            
                current->current_time = next->current_time;
                next->current_time = tempCurrentTime;
            
                current->tid = next->tid;
                next->tid = tempTID;
            
                //current->remaining_time = next->remaining_time;
                //next->remaining_time = tempRemainingTime;

            }
        } 
        
        current = current->next;
        next = next->next;
      }
   }   
}

int *createCondList(int total_threads) {
    thread_count = total_threads;
    
    condListCPU = (struct pthread_cond_data *)malloc(thread_count * sizeof *condListCPU);
    condListIO = (struct pthread_cond_data *)malloc(thread_count * sizeof *condListIO);
    condListPV = (struct pthread_cond_data *)malloc(thread_count * sizeof *condListPV);
    
    semList = (struct semaphoreData *)malloc(thread_count * sizeof *semList);
    
    for(int i = 0; i < thread_count; i++) condListCPU[i].tid = i; 
    for(int i = 0; i < thread_count; i++) condListIO[i].tid = i; 
    
    for(int i = 0; i < thread_count; i++) {
        semList[i].semaphore = 0; 
        semList[i].semID = i;
    }
    
    return 0;
}

void enqueueCPU(float current_time, int tid, int remaining_time) {
    struct node *newNode = malloc(sizeof(struct node));
    newNode->current_time = current_time;
    newNode->tid = tid;
    newNode->remaining_time = remaining_time;
    newNode->next = NULL;
    //condListCPU[count].tid = tid;
    pthread_cond_init(&(condListCPU[tid].thread_cond),NULL);

    if(headCPU == NULL && tailCPU == NULL)
        headCPU = tailCPU = newNode;
    else
    {
        tailCPU->next = newNode;
        tailCPU = newNode;
    }
    count++;
    sortCPU();
}

void dequeueCPU()
{
    struct node *temp;
    //condListCPU[getCPUFirstTid()].thread_cond = NULL; //New LIne
    if(headCPU != NULL){
        temp = headCPU;
        headCPU = headCPU->next;
        count--;

        if(headCPU == NULL)
            tailCPU = NULL;

       free(temp);
    }
}

void enqueueIO(float current_time, int tid) {
    struct node *newNode = malloc(sizeof(struct node));
    newNode->current_time = current_time;
    newNode->tid = tid;

    //newNode->remaining_time = remaining_time;
    newNode->next = NULL;
    pthread_cond_init(&(condListIO[tid].thread_cond),NULL);

    if(headIO== NULL && tailIO == NULL)
        headIO = tailIO = newNode;
    else
    {
        tailIO->next = newNode;
        tailIO = newNode;
    }
    countIO++;
    sortIO();
}

void dequeueIO() {
    struct node *temp;

    if(headIO != NULL){
        temp = headIO;
        headIO = headIO->next;
        countIO--;

        if(headIO == NULL){
            tailIO = NULL;
        }
        
       free(temp);
    }
}

void printCPUList(){
    temp = headCPU;
    for(int i = 0; i <count; i++){
        printf("(%.1f,%d,%d),%d, %p\n", temp->current_time, temp->tid, temp->remaining_time, condListCPU[i].tid, &condListCPU[i].thread_cond);
        temp = temp->next;
    }
}

void printIOList(){
    temp = headIO;
    for(int i = 0; i < countIO; i++){
        printf("(%.1f,%d),%d, %p\n", temp->current_time, temp->tid, condListIO[i].tid, &condListIO[i].thread_cond);
        temp = temp->next;
    }
}
void decrementRemainingTime(){
    headCPU->remaining_time--;
}

float getFirstCurrentTime(){
    return headCPU->current_time;
}

int getCPUFirstTid(){
    return headCPU->tid;
}

int getIOFirstTid(){
    return headIO->tid;
}
int getFirstRemainingTime(){
    return headCPU->remaining_time;
}

pthread_cond_t* getCPUThreadCond(int tid){
    /*for(int i = 0; i < thread_count; i++){
        if(condListCPU[i].tid == tid){
            return &condListCPU[i].thread_cond;
        }
    }*/
    return &condListCPU[tid].thread_cond;
    //return NULL;
}

pthread_cond_t* getIOThreadCond(int tid){
    
    /*for(int i = 0; i < thread_count; i++){
        if(condListIO[i].tid == tid){
            return &condListIO[i].thread_cond;
        }
    }
    return NULL;*/
    return &condListIO[tid].thread_cond;
}


int getRemainingTime(int tid){
    int temp_time;
    temp = headCPU;
    for(int i=0; i<count; i++){
        if (temp->tid==tid){
	    temp_time = temp->remaining_time;
            return temp_time;
        }
        temp = temp->next;
    }
}

int checkExistenceCPU(int tid) {
    int exist = 0; //Does not exist in the queue
    temp = headCPU;
    for(int i=0; i<count; i++){
        if (temp->tid==tid){
	    exist++;
            return exist; //Does exist in the queue
        }
        temp = temp->next;
    }
    return exist;
}

int checkExistenceIO(int tid) {
    int exist = 0; //Does not exist in the queue
    temp = headIO;
    for(int i=0; i<count; i++){
        if (temp->tid==tid){
	    exist++;
            return exist; //Does exist in the queue
        }
        temp = temp->next;
    }
    return exist;
}

void decrementSem(int sem_id){  // For P
    semList[sem_id].semaphore--;
}

void incrementSem(int sem_id){ // For V
    semList[sem_id].semaphore++;
}

int getSem(int sem_id){
    return semList[sem_id].semaphore;
} 



// OLD IMPLEMENTATION ///////////
/*
struct node
{
    float current_time;
    int tid;
    int remaining_time;
    pthread_cond_t thread_cond;
    struct node *next;
};

typedef struct pthread_cond_data {
    int tid;
    pthread_cond_t thread_cond;
} new_condition;


// For CPU_ME
int count = 0;
int countIO = 0;
int thread_count = 0;
struct node *headCPU = NULL, *tailCPU = NULL, *temp = NULL;
static new_condition *condListCPU, *condListIO;
 
// For IO_ME
struct node *headIO = NULL, *tailIO = NULL;// *ioTemp = NULL;
//pthread_cond_t *temp_cond;


void specificDequeueCPU(int tid){
    struct node *temp1 = headCPU, *prev;
    //condListCPU[tid].thread_cond = NULL;
    if (temp1 != NULL && temp1->tid == tid) {
        headCPU = temp1->next; 
        free(temp1);
	return;
    }
    while (temp1 != NULL && temp1->tid != tid) {
        prev = temp1; 
        temp1 = temp1->next;
    }

    if (temp1 == NULL){
        return;
    }
 
    prev->next = temp1->next;
    free(temp1);
    
}


void sortCPU() {

   int i, j, k, tempTID, tempRemainingTime;
   float tempCurrentTime;
   //pthread_cond_t tempThreadCondition;
   struct node *current;
   struct node *next;
	
   //int size = length();
   k = count;
	
   for ( i = 0 ; i < count - 1 ; i++, k-- ) {
      current = headCPU;
      next = headCPU->next;
		
      for ( j = 1 ; j < k ; j++ ) {   

        if ( current->current_time > next->current_time ) {
            tempCurrentTime = current->current_time;
            tempTID = current->tid;
            tempRemainingTime = current->remaining_time;
            
            //tempThreadCondition = current->thread_cond;
            
            current->current_time = next->current_time;
            next->current_time = tempCurrentTime;
            
            current->tid = next->tid;
            next->tid = tempTID;
            
            current->remaining_time = next->remaining_time;
            next->remaining_time = tempRemainingTime;
            
        }
        
        else if(current->current_time == next->current_time){
            if(current->tid > next->tid){
                tempCurrentTime = current->current_time;
                tempTID = current->tid;
                tempRemainingTime = current->remaining_time;
                
                //tempThreadCondition = current->thread_cond;
            
                current->current_time = next->current_time;
                next->current_time = tempCurrentTime;
            
                current->tid = next->tid;
                next->tid = tempTID;
            
                current->remaining_time = next->remaining_time;
                next->remaining_time = tempRemainingTime;

            }
        } 
        
        current = current->next;
        next = next->next;
      }
   }   
}

void sortIO() {

   int i, j, k, tempTID;
   float tempCurrentTime;
   //pthread_cond_t tempThreadCondition;
   struct node *current;
   struct node *next;
	
   //int size = length();
   k = countIO;
	
   for ( i = 0 ; i < countIO - 1 ; i++, k-- ) {
      current = headIO;
      next = headIO->next;
		
      for ( j = 1 ; j < k ; j++ ) {   

        if ( current->current_time > next->current_time ) {
            tempCurrentTime = current->current_time;
            tempTID = current->tid;
            //tempRemainingTime = current->remaining_time;
            
            //tempThreadCondition = current->thread_cond;
            
            current->current_time = next->current_time;
            next->current_time = tempCurrentTime;
            
            current->tid = next->tid;
            next->tid = tempTID;
            
            //current->remaining_time = next->remaining_time;
            //next->remaining_time = tempRemainingTime;
            
        }
        
        else if(current->current_time == next->current_time){
            if(current->tid > next->tid){
                tempCurrentTime = current->current_time;
                tempTID = current->tid;
                //tempRemainingTime = current->remaining_time;
                
                //tempThreadCondition = current->thread_cond;
            
                current->current_time = next->current_time;
                next->current_time = tempCurrentTime;
            
                current->tid = next->tid;
                next->tid = tempTID;
            
                //current->remaining_time = next->remaining_time;
                //next->remaining_time = tempRemainingTime;

            }
        } 
        
        current = current->next;
        next = next->next;
      }
   }   
}

int *createCondList(int total_threads) {
    thread_count = total_threads;
    condListCPU = (struct pthread_cond_data *)malloc(thread_count * sizeof *condListCPU);
    condListIO = (struct pthread_cond_data *)malloc(thread_count * sizeof *condListIO);
    for(int i = 0; i < thread_count; i++) condListCPU[i].tid = i; //New line
    for(int i = 0; i < thread_count; i++) condListIO[i].tid = i; //New line
    return 0;
}

void enqueueCPU(float current_time, int tid, int remaining_time) {
    struct node *newNode = malloc(sizeof(struct node));
    newNode->current_time = current_time;
    newNode->tid = tid;
    newNode->remaining_time = remaining_time;
    newNode->next = NULL;
    //condListCPU[count].tid = tid;
    pthread_cond_init(&(condListCPU[tid].thread_cond),NULL);

    if(headCPU == NULL && tailCPU == NULL)
        headCPU = tailCPU = newNode;
    else
    {
        tailCPU->next = newNode;
        tailCPU = newNode;
    }
    count++;
    sortCPU();
}

void dequeueCPU()
{
    struct node *temp;
    //condListCPU[getCPUFirstTid()].thread_cond = NULL; //New LIne
    if(headCPU != NULL){
        temp = headCPU;
        headCPU = headCPU->next;
        count--;

        if(headCPU == NULL)
            tailCPU = NULL;

       free(temp);
    }
}

void enqueueIO(float current_time, int tid) {
    struct node *newNode = malloc(sizeof(struct node));
    newNode->current_time = current_time;
    newNode->tid = tid;

    //newNode->remaining_time = remaining_time;
    newNode->next = NULL;
    pthread_cond_init(&(condListIO[tid].thread_cond),NULL);

    if(headIO== NULL && tailIO == NULL)
        headIO = tailIO = newNode;
    else
    {
        tailIO->next = newNode;
        tailIO = newNode;
    }
    countIO++;
    sortIO();
}

void dequeueIO() {
    struct node *temp;

    if(headIO != NULL){
        temp = headIO;
        headIO = headIO->next;
        countIO--;

        if(headIO == NULL){
            tailIO = NULL;
        }
        
       free(temp);
    }
}

void printCPUList(){
    temp = headCPU;
    for(int i = 0; i <count; i++){
        printf("(%.1f,%d,%d),%d, %p\n", temp->current_time, temp->tid, temp->remaining_time, condListCPU[i].tid, &condListCPU[i].thread_cond);
        temp = temp->next;
    }
}

void printIOList(){
    temp = headIO; 
    for(int i = 0; i < countIO; i++){
        printf("(%.1f,%d),%d, %p\n", temp->current_time, temp->tid, condListIO[i].tid, &condListIO[i].thread_cond);
        temp = temp->next;
    }
}
void decrementRemainingTime(){
    headCPU->remaining_time--;
}

float getFirstCurrentTime(){
    return headCPU->current_time;
}

int getCPUFirstTid(){
    return headCPU->tid;
}

int getIOFirstTid(){
    return headIO->tid;
}
int getFirstRemainingTime(){
    return headCPU->remaining_time;
}

/*void deleteCond(int tid)
{
   int index = 0;
   for(int i = 0; i < thread_count; i++) {
   	if(condListCPU[i].tid == tid) index = i;
   }
   for(int i = index; i < thread_count - 1; i++) condList[i] = array[i + 1];
   for
}
pthread_cond_t* getCPUThreadCond(int tid){
    /*for(int i = 0; i < thread_count; i++){
        if(condListCPU[i].tid == tid){
            return &condListCPU[i].thread_cond;
        }
    }
    return &condListCPU[tid].thread_cond;
    //return NULL;
}

pthread_cond_t* getIOThreadCond(int tid){
    
    /*for(int i = 0; i < thread_count; i++){
        if(condListIO[i].tid == tid){
            return &condListIO[i].thread_cond;
        }
    }
    return NULL;
    return &condListIO[tid].thread_cond;
}

int getRemainingTime(int tid){
    int temp_time;
    temp = headCPU;
    for(int i=0; i<count; i++){
        if (temp->tid==tid){
	    temp_time = temp->remaining_time;
            return temp_time;
        }
        temp = temp->next;
    }
}

int checkExistenceCPU(int tid) {
    int exist = 0; //Does not exist in the queue
    temp = headCPU;
    for(int i=0; i<count; i++){
        if (temp->tid==tid){
	    exist++;
            return exist; //Does exist in the queue
        }
        temp = temp->next;
    }
    return exist;
}

int checkExistenceIO(int tid) {
    int exist = 0; //Does not exist in the queue
    temp = headIO;
    for(int i=0; i<count; i++){
        if (temp->tid==tid){
	    exist++;
            return exist; //Does exist in the queue
        }
        temp = temp->next;
    }
    return exist;
}
*/







