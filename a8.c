#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

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
   void domd5(char *path);
   void usage(char *progname);
   int chkdst(char **argv);
   void die(char *why);
   long int filesize(char *srcpath);
   void buildpath(char *src, char *dst, char **dstpath);
   int isvalid(char *path, char *dst);
   void *dowork(void *arg);

   if (argc < 4) usage("a8");
   int workers, i;
   char *check; 
   workers = strtol(argv[3], &check, 10);
   if (!check) usage("a8");
   else if (!chkdst(&argv[2])) die ("DST not valid!");
   long int size = filesize(argv[1]);
   if (size == -1) die ("Could not find file size");
   char *dstpath; buildpath(argv[1], argv[2], &dstpath);
   if (!isvalid(argv[1], dstpath)) die ("scr not valid!");
   long int chunksize = size / workers;
   long int remainder = size % workers;
   
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

void usage(char *progname)
{
   fprintf(stderr, "./%s srcpath dstpath workercount\n", progname);
   exit(0);
}

void die(char *why)
{
   fprintf(stderr, "Program Killed...\nReason: %s\n", why);
   exit(1);
}

long int filesize(char *srcpath)
{
   struct stat st;
   if(stat(srcpath, &st) != 0) return 0;
   return st.st_size;
}

void domd5(char *path)
{
   pid_t pid = fork();
   if (pid == 0) 
      execlp("md5", "md5", path, NULL);
   wait(0);
}

void *dowork(void *arg)
{
   threadarg *args = (threadarg *)arg;
   int fd = args->fd, 
   copy = args->copy, rd;
   long int start = args->start, 
   size = args->size;
   char bufs[2048], *remains;
   barrier--;
   while (barrier > 0);
   lseek(fd, start, SEEK_SET);
   lseek(copy, start, SEEK_SET);

   long int count = 0, remainder = 0, i;
   for (i = 0; i < size; i += 2048)
   {
      if (i + 2048 > size)
      {
         remainder = size - count;
         remains = malloc(remainder * sizeof(char));
         rd = read (fd, remains, sizeof(remains));
         if (write(copy, remains, rd) != rd)
            die("Error accessing  files during copy"); 
         count += remainder;
      }
      else
      {
         rd = read(fd, bufs, sizeof(bufs));
         if (write(copy, bufs, rd) != rd)
            die("Error accessing files during copy");  
         count += 2048;
      }    
   }
   close(fd); close(copy);
   
   printf("%d thread with offset %ldKB, reached barrier\n", (int) pthread_self(), start);
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
   {
      return 0;
   }
   else if (!isregular(path))
   {
      return 0;
   }
   else if (dst == NULL)
   {
      return 0;
   }
   else if (isregular(dst))
   {
      return 0;
   }

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
