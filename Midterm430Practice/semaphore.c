#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
//#include <windows.h>
#define NUMTHREADS 9
#define NUMGPUS 3
#define RUNTIME 5
// pthread*
// sem*
// printf
// clock
// sleep
// Using Semaphores
// millisecs
pthread_mutex_t printMutex; // avoid printf interleaving
sem_t gpuSem;               // access to GPUs

void Report(int tid, char *msg) {
  pthread_mutex_lock(&printMutex);
  clock_t now = clock();
  printf("Tid=%d clock=%06d \t %s \n", tid, now, msg);
  pthread_mutex_unlock(&printMutex);
}

void *gpu(void *arg) {
  int me = pthread_self(); // what is my TID?
  Report(me, "Requesting GPU");
  sem_wait(&gpuSem);
  Report(me, "Running Running Running");
  sleep(RUNTIME);
  Report(me, "Finished");
  sem_post(&gpuSem);
  return NULL;
}

int main() {
  pthread_mutex_init(&printMutex, NULL);
  sem_init(&gpuSem, 0, NUMGPUS);
  pthread_t tids[NUMTHREADS];
  for (int t = 0; t < NUMTHREADS; ++t) {
    pthread_create(&tids[t], NULL, gpu, NULL);
  }
  for (int t = 0; t < NUMTHREADS; ++t) {
    pthread_join(tids[t], NULL);
  }
  return 0;
}