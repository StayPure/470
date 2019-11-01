/*
	Name: Bell, Berman		                      Class: CPS 470
   	Section: 22371841                      Assignment: 06
   	Due: November 1, 2019                  Started: October 24, 2019
   	Credit: 10 points.
	
 Problem: Modify the given code in psr.c to simulate a binary
 semaphore using a pipe. The program should implement the functions
 seminit(), semap(), semav, and semrel(). The provided code should
 be modified to eliminate the present race condition.
	
 Solution: The program initializes a semaphore and performs the 
 necessary P and C operations on the semaphore, reading/writing
 from/to the pipe as required. It accomplishes this by creating
 the thread and using pthread_join() to allow threads to wait for
 another thread to terminate. The operations in the *term() function
 carry out the necessary operations. Once complete, the read and write 
 ends of the pipe are closed.
	
 Data-structure used: A one dimensional array for the value of
 s passed to the semaphore.
 
	Accessing functions for the data structure: Standard C functions
	for accessing arrays.
	
 Errors handled: The program checks if the semaphore was initialized
 properly as well as if there are any errors in the P and V operations.
 Also checks if there is an error when closing the respective ends of the 
 pipe.
	
 Limitations: The pipe must be created before the threads. 
 The descriptors must be global.
	
 Acknowledgment: We wrote the program together but we had a lot of
	help from our classmates throughout the making of it.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

int s[2];

/* Access shared resource stderr */

/*
  seminit() initializes the semaphore by initializing the pipe
  and writing a 1 to the write-end of the pipe. Checks if there
  are failures in the creation of the pipe and writing to the pipe.
  In the event of a failure, 0 is returned. On success, 1 is returned.
*/
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


/*
  semap() performs a p operation on the simulated semaphore
  by reading a character from the read end of the pipe. If the 
  read operation does not succeed, 0 is returned. Otherwise, 1
  is returned.
*/
int semap()
{
   int x;
   if (read(s[0], &x, sizeof(x)) == -1) return 0;
   return 1;
}

/*
  semav() performs a v operation on the simulated semaphore
  by writing a 1 to the write end of the pipe. if the write
  operation does not succeed, 0 is returned. Otherwise 1 is
  returned.
*/
int semav()
{
   int x = 1;
   if (write(s[1], &x, sizeof(x)) == -1) return 0;
   return 1;
}

/*
  semrel() closes the read and write ends of the pipe.
  Returns 0 if closing either end of the pipe fails.
  Otherwise returns 1.
*/
int semrel()
{
   if(close(s[0]) == -1) return 0;
   if(close(s[1]) == -1) return 0;

   return 1;
}

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
