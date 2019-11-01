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

int s[2];

/* Access shared resource stderr */

int seminit()
{
   if (pipe(s) < 0) {
      fprintf(stderr, "PIPE FAIL!");
      exit(1);
   }
   int x = 1;
   if (write(s[1], &x, sizeof(x)) == -1) return 0;
   return 1;
}

int semap()
{
   int x;
   if (read(s[0], &x, sizeof(x)) == -1) return 0;
   return 1;
}

int semav()
{
   int x = 1;
   if (write(s[1], &x, sizeof(x)) == -1) return 0;
   return 1;
}

int semrel()
{
   if(close(s[0]) == -1) return 0;
   if(close(s[1]) == -1) return 0;

   return 1;
}


void *term(void *arg)
{
   int i, n = *(int *)arg;
   if (!semap()) {printf("ERROR ON P OPERATION\n"); exit(1);}
   for(i = 0; i < n; i++) {
      if(n == 4) pthread_yield();
      fprintf(stderr, " %u ", (int) pthread_self());
   }
   if (!semav()) {printf("ERROR ON V OPERATION\n"); exit(1);}

   pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
   pthread_t tid[10];
   int k, n;
   seminit();

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

   if (!semrel()) {fprintf(stderr, "CLOSING FAILURE"); exit(1);}
   exit(0);
}
