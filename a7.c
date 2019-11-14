/*
	Name: Bell, Berman		                   Class: CPS 470
   	Section: 22371841                                  Assignment: 07
   	Due: November 14, 2019                  Started: November 4, 2019
   	Credit: 10 points.
    
    Problem: Write a fully-functional program that implements a bounded-
    buffer producer/consumer problem using POSIX threads to implement
    multiple producer and consumer threads. The threads must synchronize
    their activity using binary and counting semaphores. The program 
    should check that arguments from the command line satisfy the 
    necessary contraints. When an item is moved to/from the buffer,
    the corresponding thread should print a message to stdout stating
    the thread id, item moved, and the location to/from which it is 
    moved.
    
    Solution: A struct queue is defined to act as the bounded buffer
    and functions as a circular queue. The arguments from the 
    command line are checked in chkargs() to verify that they
    are valid. If invalid, the function calls usage() or die() as
    required. Otherwise returns 1. The semaphore is then initialized
    by init(). Once the threads are created, they synchronously 
    perform the operations in the producer and consumer modules
    corresponding to the nature of the thread. Once complete, the
    event and each thread are seen in stdout in the proper format.
    The destroy module is then called to close the semaphore and the 
    program ends.
    
    Data-structures used: User-defined struct queue to act as the bounded 
    buffer, one-dimensional array.
    
    Accessing functions for the data structure: Standard C functions for 
    accessing arrays.
    
    Limitations: A producer cannot place an item in the buffer if there is
    no room in the buffer. Likewise, a consumer cannot consume from the 
    buffer if the buffer is empty.
    
    Acknowledgment: We both worked on this program together, but had help
    from our peers.
*/

#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#define BFSIZE 301            /* why 301?? */
#define MXTHRDS 380
                
typedef struct queue {        /* viewed as a circular queue */
  char data[BFSIZE];
  int front, rear;
  sem_t emptyslots, fullslots, placeitem, removeitem;
} BUFFER;
BUFFER buffer;

int main(int argc, char *argv[])
{ 
   int chkargs(int argc, char *argv[],  int *prods, int *proditers, int *consmrs, int *criters);
   void *consumer(void *arg);
   void destroy(); void die(char *why);
   void *producer(void *arg);
   int init();

   int prods = 0, proditers = 0, consmrs = 0, criters = 0, i;
   chkargs(argc, argv, &prods, &proditers, &consmrs, &criters);
   pthread_t producers[prods], consumers[consmrs];
   int piters[prods], citers[consmrs];
   init();

   for(i = 0; i < prods; i++)
   {
      piters[i] = proditers;
      if(pthread_create(&producers[i], NULL, producer, &piters[i]))
         die("Thread Creation Failure");
   }

   for(i = 0; i < consmrs; i++)
   {
      citers[i] = criters;
      if (pthread_create(&consumers[i], NULL, consumer, &citers[i]))
         die("Thread Creation Failure");
   }

   for (i = 0; i < prods; ++i)
      pthread_join(producers[i], NULL);

   for (i = 0; i < consmrs; i++)
      pthread_join(consumers[i], NULL);

   destroy();
   exit(0);
}

/*
   Takes progname and prints out the correct usage of the program
   to stderr then exits the program with a failure.
   Called by chkargs()
*/
void usage(char *progname)
{
  fprintf(stderr,
   "usage: ./%s <# of producers> <producer interations> <# of consumers> <consumer iterations>\n", progname);
  exit(1);
}

/*
   Takes the args from the command line and performs a sanity check.
   *continued later*
*/
int chkargs(int argc, char *argv[], int *prods, int *proditers, int *consmrs, int *criters)
{
   void die(char *why);

   if (argc != 5) usage("prodcons");
   char *check;
   *prods = strtol(argv[1], &check, 10); 
   if (!check) usage("prodcons");
   *consmrs = strtol(argv[3], &check, 10);
   if (!check) usage("prodcons");
   *proditers = strtol(argv[2], &check, 10); 
   if (!check) usage("prodcons");
   *criters = strtol(argv[4], &check, 10);
   if (!check) usage("prodcons");

   if ((*proditers * *prods) != (*criters * *consmrs))
      die("proditers x nrprods != consiters x nrcons");
   else if ((*prods + *consmrs) > MXTHRDS)
      die("Exceed the number of max threads");
   else if (*prods <= 0)
      die ("No producers!");
   else if (*consmrs <= 0)
      die ("No consumers!");

   return 1;
}

void *producer(void *arg)
{
   int enq(char item);
   char randchar();
   void die(char *why);
   int *numiters = arg;

   int i = 0;
   for (i = 0; i < *numiters; i++)
   {
      if (sem_wait(&buffer.emptyslots) == -1) die("Semaphore Wait Faliure");
      if (sem_wait(&buffer.placeitem) == -1) die("Semaphore Wait Faliure");

      char item =  randchar();
      enq(item);
      fprintf(stderr, "%u: produced: %c, placed in buffer: %d\n", 
      (int) pthread_self(), item, buffer.rear);

      if (sem_post(&buffer.placeitem) == -1) die("Semaphore Post Faliure");
      if (sem_post(&buffer.fullslots) == -1) die("Semaphore Post Faliure");
      
   }

   pthread_exit(NULL);
}

void *consumer(void *arg)
{
   int deq(char *item);
   void die(char *why);
   int *numiters = arg;

   int i = 0;
   for (i = 0; i < *numiters; i++)
   {
      if (sem_wait(&buffer.fullslots) == -1) die("Semaphore Wait Faliure");
      if (sem_wait(&buffer.removeitem) == -1) die("Semaphore Wait Faliure");

      char item = buffer.data[buffer.front];
      int front = buffer.front;
      deq(&buffer.data[buffer.front]);
      fprintf(stderr, "%u: consumed: %c, removed from buffer: %d\n", 
      (int) pthread_self(), item, front);

      if (sem_post(&buffer.removeitem) == -1) die("Semaphore Post Faliure"); 
      if (sem_post(&buffer.emptyslots) == -1) die("Semaphore Post Faliure");
   }
   pthread_exit(NULL);
}


char randchar()
{
   return (char) (rand() % 26) + 65;
}

void die(char *why)
{
   fprintf(stderr, "Program Killed...\nReason: %s\n", why);
   exit(1);
}

int enq(char item)
{
   buffer.data[buffer.rear] = item;
   if (buffer.rear == BFSIZE - 1) buffer.rear = 0;
   else buffer.rear++;
   return 1;
}

int deq(char *item)
{ 
   *item = '_';
   if (buffer.front == BFSIZE - 1) buffer.front = 0;
   else buffer.front++;
   return 1;
}

int init()
{
   if (sem_init(&buffer.emptyslots, 1, BFSIZE) == -1
   || sem_init(&buffer.fullslots, 1, BFSIZE) == -1
   || sem_init(&buffer.placeitem, 1, 1) == -1
   || sem_init(&buffer.removeitem, 1, 1) == -1)
      die("Semaphore Initialization Failure");
   
   int i;
   for (i = 0; i < BFSIZE; i++)
   {
      if (sem_wait(&buffer.fullslots) == -1)
         die("Buffer Full Slots Initialization Failure");
      buffer.data[i] = '_';
   }   

   buffer.front = 0;
   buffer.rear = 0;
   return 0;
}

void destroy()
{
   if (sem_destroy(&buffer.emptyslots) == -1
   || sem_destroy(&buffer.fullslots) == -1
   || sem_destroy(&buffer.placeitem) == -1
   || sem_destroy(&buffer.removeitem) == -1)
      die("Semaphore Closure Failure");
}
