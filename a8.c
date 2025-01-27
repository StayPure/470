/*
	Name: Bell, Berman		                              Class: CPS 470
   	Section: 22371841                                  Assignment: 08
   	Due: November 26, 2019                  Started: November 14, 2019
   	Credit: 10 points.
    
    Problem: Write a fully-functional program that copies a given source
    file to a destination file using multiple worker threads to perform
    the copying activity. The source path, destination path, and number
    of threads are to be specified by the command line arguments. The
    number of threads provided will determine the number of chunks that
    the source file is divided in to and all chunks except the last chunk
    will be of the same size.
    
    Solution: A struct args is defined in order to pass the arguments to the
    respective worker thread. Threads are created and the file size is 
    defined as well as the offset of the chunks. The files are then opened
    in the required mode and checked for vailidity. The program prints out
    the id and offest of the thread when it reaches the barrier. After all
    threads reach the barrier, the md5sum of the destination path is printed.
    
    Data-structures used: User-defined struct args used to pass arguments
    to the worker threads.
    
    Accessing functions for the data structure: Standard C functions for 
    accessing arrays.
    
    Limitations: The program must use POSIX threads. The source path, 
    destination path, and number of threads to use must be specified
    in the command lines.
    
    Acknowledgment: We both worked on this program together, but had help
    from our peers.
*/
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>

typedef struct args {
   int fd;
   int copy;
   int turn;
   long int start;
   long int size;
}threadarg;

int barrier = 0;

int main(int argc, char *argv[])
{
   void buildpath(char *src, char *dst, char **dstpath);
   int chkdst(char **argv);
   void die(char *why);
   void domd5(char *path);
   void *dowork(void *arg);
   long int filesize(char *srcpath);
   int isvalid(char *path, char *dst);
   void usage(char *progname);

   if (argc < 4) usage("a8");

   char *check;
   int workers = strtol(argv[3], &check, 10), i;
   if (!check) usage("a8");
   else if (!chkdst(&argv[2])) die ("DST not valid!");

   long int size = filesize(argv[1]);
   if (!size) die ("Could not find file size");

   char *dstpath; buildpath(argv[1], argv[2], &dstpath);
   isvalid(argv[1], dstpath);

   long int chunksize = size / workers, remainder = size % workers;
   barrier = workers;
   threadarg threadargs[workers];
   pthread_t threads[workers];

   domd5(argv[1]);
   for (i = 0; i < workers; i++)
   {
      threadargs[i].fd = open(argv[1], O_RDONLY);
      threadargs[i].copy = open(dstpath, O_CREAT | O_RDWR, 0644);
      
      threadargs[i].start = i * chunksize;
      if (i == workers - 1) 
         threadargs[i].size = chunksize + remainder;
      else 
         threadargs[i].size = chunksize;

      if (pthread_create(&threads[i], NULL, dowork, (void *) &threadargs[i]))
         die("Thread Creation Failure");
   }
   
   for (i = 0; i < workers; i++)
      pthread_join(threads[i], NULL);
   
   domd5(dstpath);
}

/*
   Takes progname and prints out the correct usage of the program
   to stderr then exits the program with a failure.
   Called by main()
*/
void usage(char *progname)
{
   fprintf(stderr, "./%s srcpath dstpath workercount\n", progname);
   exit(0);
}

/*
   Takes why and prints out the reasonthe program was killed
   to stderr then exits the program with a failure.
   Called by main().
   Called by dowork().
*/
void die(char *why)
{
   fprintf(stderr, "Program Killed...\nReason: %s\n", why); exit(1);
}

/*
   Takes in the source path and returns the size of the file.
   If the process fails, 0 is returned.
   Called by main().
*/
long int filesize(char *srcpath)
{
   struct stat st;
   if(stat(srcpath, &st) != 0) return 0;
   return st.st_size;
}

/*
   Takes in the path and creates a child using fork() to execute
   md5sum. This is accomplished by using the execlp() system call.
   Called by main().
*/
void domd5(char *path)
{
   printf("\n");
   pid_t pid = fork();
   if (pid == 0) 
      execlp("md5", "md5", path, NULL);
   wait(0);
   printf("\n");
}

/*
   Takes in the arguments passed in using the user-defined
   struct. The function is executed by each worker thread.
   Starts the copy designated for the thread and prints the
   relevant output. Also checks if there was an error accessing
   files during the copy. In this case, the function calls die() 
   with the respective error message. When complete, the thread exits.
   Called by main().
   Calls die().
*/
void *dowork(void *arg)
{
   threadarg *args = (threadarg *)arg;
   int fd = args->fd, copy = args->copy, rd;
   long int start = args->start, size = args->size;
   char bufs[2048];
   lseek(fd, start, SEEK_SET); lseek(copy, start, SEEK_SET);

   barrier--;
   printf("%d thread with offset %ldKB, reached barrier\n", 
   (int) pthread_self(), start);
   while (barrier > 0);

   int i;
   for (i = 0; i < size; i+= 2048)
   {
      rd = read(fd, bufs, sizeof(bufs));
         if (write(copy, bufs, rd) != rd)
            die("Error accessing files during copy");    
   }

   close(fd); close(copy);
   pthread_exit(NULL);
}

/* Takes a single pointer, *argv, and passes it to isdir()
   to check if the directory exists. If isdir returns a 1 a
   1 is returned from this module. Otherwise, an error message
   is printed and a 0 is returned.
   Calls isdir().
   Called by main().
*/
int chkdst(char **argv)
{
   int isdir(char *path);

   if (isdir(*argv)) return 1;
   return 0;
}

/* Uses the Stat struct to construct a struct, sbuf,
   and uses stat() to obtain information from the file and
   write it to sbuf. Uses S_ISDIR() on sbuf.st_mode to see
   the mode of the file. A 1 is returned if the file is a
   directory otherwise a 0 is returned.
   Called by isvalid().
*/
int isdir(char *path)
{
   struct stat sbuf;
   if (stat(path, &sbuf)) return 0;
   return S_ISDIR(sbuf.st_mode);
}

/* Uses the Stat struct to construct a struct, sbuf,
   and uses stat() to obtain information from the file and
   write it to sbuf. Uses S_ISREG on sbuf.st_mode to see if
   the file is regular. A 1 is returned if the S_ISREG is true
   otherwise a 0 is returned.
   Called by isvalid().
*/
int isregular(char *path)
{
   struct stat sbuf;
   if (stat(path, &sbuf)) return 0;
   return S_ISREG(sbuf.st_mode);
}

/* Checks if the source path is a directory first, then if its
   a regular file return 0 if it is dir and if it isn't a regular
   file, then checks if the destionation path was created or if
   the file exist at the destination if either return 0, if none
   of these return 1.
   Calls isdir() and isregular().
   Called by copyfiles().
*/
int isvalid(char *path, char *dst)
{
   if (isdir(path)) 
      die(strcat(path, "  is not a regular file"));
   else if (!isregular(path)) 
      die(strcat(path, "  doesn't exist..."));
   else if (dst == NULL) 
      die ("Destination creation failure");
   else if (isregular(dst)) 
      die (strcat(path, "  already exist at location!"));

   return 1;
}

/* Builds destination-path using strrchr() function from library,
   dstpath is null on error and defined otherwise. The src file has
   its original destination removed and replaced with the new one if
   it has a original destination on it otherwise it is just added to
   the end of the existing name of the file.
   Called by copyfiles().
*/
void buildpath(char *src, char *dst, char **dstpath)
{
   char *ptr;
   int n;
   ptr = strrchr(src, '/');

   if (ptr) n = strlen(dst) + strlen(ptr) + 2;
   else n = strlen(dst) + strlen(src) + 2;

   *dstpath = malloc(n);

   if (!dstpath) return;

   if (ptr)
   {
      strcpy(*dstpath, dst);
      strcat(*dstpath, ptr);
   }
   else
   {
      strcpy(*dstpath, dst);
      strcat(*dstpath, "/");
      strcat(*dstpath, src);
   }
}
