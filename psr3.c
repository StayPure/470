/*
	Name: Bell, Berman		                      Class: CPS 470
   	Section: 22371841                      Assignment: 06
   	Due: November 1, 2019                  Started: October 24, 2019
   	Credit: 10 points.
	
 Problem: Modify the given code in psr.c to simulate a binary
 semaphore using a binary semaphore from the POSIX library.
 The provided code should be modified to eliminate the present race 
 condition.
	
 Solution: The program initializes a semaphore and performs the 
 necessary P and C operations on the semaphore by using semwait() and
 sempost(). It accomplishes this by creating
 the thread and using pthread_join() to allow threads to wait for
 another thread to terminate. The operations in the *term() function
 carry out the necessary operations. Once complete, the read and write 
 ends of the pipe are closed.
	
 Data-structure used: A one dimensional array for pthread_t tid.
 
	Accessing functions for the data structure: Standard C functions
	for accessing arrays.
	
 Errors handled: The program checks if the thread join and creation fail
 as well as if there are any errors in the P and V operations.
 Also checks if there is an error when closing by using semdestroy().
	
 Limitations: The program must use the binary semaphore from the POSIX
 library.
	
 Acknowledgment: We wrote the program together but we had a lot of
	help from our classmates throughout the making of it.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

sem_t sem;

/*Access shared resource stderr */

/*
  The function carries out the operations required
  for the simulated semaphore to function using threads. 
  The function exits in an error if the p or v operations
  fail. Otherwise, the operations complete successfully and the 
  thread exits.
*/
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
