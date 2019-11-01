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

pthread_mutex_t lock;
pthread_cond_t done;

/* Access shared resource stderr */

void *term(void *arg)
{
   int i, n = *(int *)arg;

   pthread_mutex_lock(&lock);
   for(i = 0; i < n; i++) {
      if(n == 4) pthread_yield();
      fprintf(stderr, " %u ", (int) pthread_self());
   }
   pthread_mutex_unlock(&lock);
   pthread_cond_signal(&done);

   pthread_exit(NULL);
}

void die (char *x)
{
   fprintf(stderr, "%s\n", x);
   exit(1);
}

int main(int argc, char *argv[])
{
   pthread_t tid[10];
   int k, n;

   if(pthread_mutex_init(&lock, NULL))
      die("MUTEX INITIALIZATION FAILURE");
   if(pthread_cond_init(&done, NULL))
      die("CONDITION INITIALIZATION FAILURE");


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

   exit(0);
}
