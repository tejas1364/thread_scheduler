#include "interface.h"
#include "scheduler.h"
#include <math.h>
#include <pthread.h>

// Interface implementation
// Implement APIs here...

static int total_threads = 0; //Thread_count
static int arrivedCPU_threads = 0; //CPU Threads that have arrived
static int arrivedIO_threads = 0; //IO Threads that have arrived
static int cpuInUse = 0;
static int ioInUse = 0;

int global_time = 0; //Global Time
pthread_mutex_t cpuLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ioLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t pLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t vLock = PTHREAD_MUTEX_INITIALIZER;

void init_scheduler(enum sch_type type, int thread_count)
{
  /*if(type == 0){
    //Create queue function that passes thread_count
  }
  else if(type == 1) {
    printf("inprogress");
  }
  else if(type == 2){
    printf("inprogress");
  }*/
  total_threads = thread_count;
  createCondList(total_threads);
  
}

int cpu_me(float current_time, int tid, int remaining_time) {
  //If tid == curr_tid {execute the cpu burst for one tick eand return global time} else {return 0}
  
  if(/*arrivedCPU_threads != total_threads && */checkExistenceCPU(tid) == 0){ //If the total threads havent arrived yet and the thread is not in the queue
    printf("new thread %d arrived\n", tid);
    pthread_mutex_lock(&cpuLock);
    arrivedCPU_threads++;
    enqueueCPU(current_time, tid, remaining_time);
    printf("thread %d just enqueued, does it exist: %d\n", tid, checkExistenceCPU(tid));
    while(arrivedCPU_threads != total_threads) {
      printf("thread %d arrived inside while loop\n", tid);
      printCPUList();
      pthread_cond_wait(getCPUThreadCond(tid), &cpuLock);
      if(cpuInUse == 0){
        break; 
      }
    }

    if(tid != getCPUFirstTid()){
      printf("thread %d arrived inside if loop\n", tid);
      printCPUList();
      printf("\nerror1");
      printf("\n");
      pthread_cond_signal(getCPUThreadCond(getCPUFirstTid()));
      printf("error2\n");
      pthread_cond_wait(getCPUThreadCond(tid), &cpuLock); //Possible deadlock
    }
    pthread_mutex_unlock(&cpuLock);
    printf("error3\n");
    printCPUList();
    printf("error4\n");
  }
  cpuInUse = 1;
  //int tempGlobal = global_time;
  printf("thread %d arrived at cpu burst\n", tid);
  if(getFirstRemainingTime() == 0){
    printf("thread %d has no cpu burst left, dequeing and arriving threads -1\n", tid);
    cpuInUse = 0;
    if(tid != getCPUFirstTid()){
      specificDequeueCPU(tid);
    }
    else{
      dequeueCPU();
    }
    //printf("failed after dequeue");
    arrivedCPU_threads--;
    if(arrivedCPU_threads > 0){
      printCPUList();
      pthread_cond_signal(getCPUThreadCond(getCPUFirstTid()));
      printf("%p \n", getCPUThreadCond(getCPUFirstTid()));
    }
    return global_time;
    //curr_tid = -1;
    //current_time = global_time; //added this in hopes of global time not resetting
  }
  else{
    if(current_time < global_time) {
      global_time = ceil(global_time) + 1.0;
      //tempGlobal = ceil(tempGlobal) + 1.0;
    }
    else{
      global_time = ceil(current_time + 1.0);
      //tempGlobal = ceil(current_time + 1.0);
    }
    //global_time = tempGlobal; //new line temp
    decrementRemainingTime();
    //return tempGlobal;
    return global_time;
  }
}


int io_me(float current_time, int tid, int duration)
{
  printf("tid %d arrived in io_me with %f and %d\n", tid, current_time, duration);
  //if(arrivedIO_threads != total_threads && checkExistenceIO(tid) == 0){ //If the total threads havent arrived yet and the thread is not in the queue
  pthread_mutex_lock(&ioLock);
  printf("thread %d arrived inside IO mutex\n", tid);
  arrivedIO_threads++;
  enqueueIO(current_time, tid);
  printf("thread %d just enqueued in IO, does it exist: %d\n", tid, checkExistenceIO(tid));
  while(tid != getIOFirstTid()) {
    printf("thread %d arrived inside IO while loop\n", tid);
    printIOList();
    pthread_cond_wait(getIOThreadCond(tid), &ioLock);
    //if(ioInUse == 0){
      //break; 
  }
  pthread_mutex_unlock(&ioLock);
  int returnTime = global_time;
  //ioInUse = 1;
//    }

  //ioInUse = 1;
  printf("%d", returnTime);
  if(current_time < returnTime) {
      returnTime = ceil(returnTime) + duration;
  }
  else{
      returnTime = ceil(current_time + duration);
  }
  printf("%d", returnTime);
  dequeueIO();
  arrivedIO_threads--;
  //ioInUse = 0;
  if(arrivedIO_threads > 0){
      printIOList();
      pthread_cond_signal(getIOThreadCond(getIOFirstTid()));
  }
  //pthread_cond_signal(getIOThreadCond(getIOFirstTid()));
  printf("tid %d finished io me with global time %d\n", tid, returnTime);
  return returnTime;
 
}


int P(float current_time, int tid, int sem_id){
  pthread_mutex_lock(&pLock);
  
  enqueuePV(tid, sem_id);
  decrementSem(sem_id);
  
  while(getSem(sem_id) < 0){
    pthread_cond_wait(getPVThreadCond(tid), &pLock);
    
  }
  pthread_mutex_unlock(&pLock);
  dequeuePV();
  
  return global_time;
  
  
}



int V(float current_time, int tid, int sem_id) {
  pthread_mutex_lock(&vLock);
  incrementSem(sem_id);
  
  if(getSem(sem_id) <= 0){
    pthread_cond_signal(getPVThreadCond(tid));
  }
  
  pthread_mutex_unlock(&vLock);
  
  return global_time;
}



void end_me(int tid)
{

}
