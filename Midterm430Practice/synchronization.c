#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int g_tally = 0;

void *counter(void *arg) {
  for (int i = 1; i <= 5; i++) {
    ++g_tally;
    printf("i: %d\n", g_tally);
  }
  return NULL;
}

int main() {
  pthread_t t1, t2;
  pthread_create(&t1, NULL, counter, NULL);
  pthread_create(&t2, NULL, counter, NULL);
  pthread_join(t1, NULL);
  pthread_join(t2, NULL);
  return 0;
}