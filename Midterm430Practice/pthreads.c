#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
// pthreads 1
/*int main() {
  pthread_t t1, t2;
  return 0;
}

void *fun(void *arg) {
  char *str = (char *)arg;
  double *d = malloc(sizeof(double));
  *d = atof(str);
  return (void *)d;
}
 */

// pthreads 2
/*
void *fun(void *arg) {
  char *str = (char *)arg;
  double *pd = malloc(sizeof(double));
  *pd = atof(str);
  return pd;
}

int main() {
  pthread_t tid;
  pthread_create(&tid, NULL, fun, "1.234");
  pthread_join(tid, NULL);
  return 0;
}
  */

// pthreads 3 //parent ignores the child's return value
/*
int main() {
    pthread_t tid;
    pthread_create(&tid, NULL, fun, "1.234");
    pthread_join(tid, NULL);
    return 0;
}
*/
// parent vatptures the return value from the child

/*
int main() {
  pthread_t tid;
  pthread_create(&tid, NULL, fun, "1.234");
  double *pd;
  pthread_join(tid, (void **)&pd);
  printf("Value: %f\n", *pd);
  free(pd);
  return 0;
}
*/
void *fun(void *arg) {
  char *str = (char *)arg;
  double *pd = malloc(sizeof(double));
  *pd = atof(str);
  return pd;
}

int main() {
  pthread_t tid;
  pthread_create(&tid, NULL, fun, "1.234");
  double *pdm;
  pthread_join(tid, (void **)&pdm);
  printf("Value: %f\n", *pdm);
  free(pdm);
  return 0;
}