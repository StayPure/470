/*
	Name: Bell, Berman		                      Class: CPS 470
   	Section: 22371841                      Assignment: 06
   	Due: November 1, 2019                  Started: October 24, 2019
   	Credit: 10 points.
	
 Problem: Modify the given code in psr.c to simulate a binary
 semaphore using pthread_mutex_lock(), pthread_mutex_unlock(), and
 pthread_mutex_destroy(). The provided code should be modified to 
 eliminate the present race condition.
	
 Solution: The program creates a thread and checks if the mutex and
 condition fail to initialize. If so, the program exits. Otherwise, 
 the contents of *term() lock and unlock access as required and
 performs the necessary operations for the semaphore. Once complete, the
 thread exits. pthread_join() is used to to allow threads to wait for
 another thread to terminate. The program also checks if there was a 
 failure in thread creation or join. If a failure occurs, the program
 exits with an error.
	
 Data-structure used: A one dimensional array for pthread_t tid.
 
	Accessing functions for the data structure: Standard C functions
	for accessing arrays.
	
 Errors handled: The program checks if the mutex and conidition initialized
 properly as well as if there are any errors in the thread join or creation.
 
 Limitations: pthread_mutex_lock(), pthread_mutex_unlock(), and 
 pthread_mutex_destroy().
	
 Acknowledgment: We wrote the program together but we had a lot of
	help from our classmates throughout the making of it.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

pthread_mutex_t lock;
pthread_cond_t done;

/* Access shared resource stderr */

/*
  The function carries out the operations required
  for the simulated semaphore to function using threads. 
  The function used lock and unlock as well as the signal
  to manage access and signal the completion of the
  operation. When the operations complete successfully and the 
  thread exits.
*/
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

/*
  die() prints a character parameter to stderr and the program
  exits.
*/
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
