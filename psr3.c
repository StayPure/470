/* Makefile should have LDFLAGS = -pthread
 Program creates some threads,
 created threads access a shared resource
 stderr indiscriminately --> and
 race conditions are observable at
 ./a.out 30 (did for me, you mileage may vary)!!

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

sem_t sem;

/*Access shared resource stderr */

void *term(void *arg)
{
   int i, n = *(int *)arg;
   if (sem_wait(&sem) == -1) {printf("ERROR ON P OPERATION\n"); exit(1);}
   for(i = 0; i < n; i++) {
      if(n == 4) pthread_yield();
      fprintf(stderr, " %u ", (int) pthread_self());
   }
   if (sem_post(&sem) == -1) {printf("ERROR ON V OPERATION\n"); exit(1);}

   pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
   pthread_t tid[10];
   int k, n;
   sem_init(&sem, 0, 1);

   if(argc < 2) { printf(" arg..\n"); exit(1); }
   n = atoi(argv[1]);

   for(k = 0; k < 10; k++) {
      if (pthread_create(&tid[k], NULL, term, (void *)&n) != 0) {
         fprintf(stderr, "pthread_create failed\n");
         exit(1);
      }
   }

   for(k = 0; k < 10; k++)
      if (pthread_join(tid[k], NULL) != 0)
         fprintf(stderr, "pthread_join failed\n");

   if (sem_destroy(&sem) == -1) {fprintf(stderr, "CLOSING FAILURE"); exit(1);}
   exit(0);
}
