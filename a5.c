/*
	Name: Bell, Berman		       Class: CPS 470
   	Section: 22371841                      Assignment: 04
   	Due: October 10, 2019                  Started: October 6, 2019
   	Credit: 5 points.

	Problem: Write a program that copies the given files to the
	specified destination directory using file system call interface
	(open(), close(), read(), write(), stat() etc.). The program
	should only handle regular files coming from the specified source
	paths. The file paths will be entered through command line
	arguments. The program should give the  usage of the program when
	the incorrect number of arguments are given (< 2), end before
	anything if the destination is invalid, and only copy files that
	are regular files and if they aren't stop the copy and give the
	reason for not doing the copy. The program also needs to use
	threads and shared variables to have files copied by different
   threads.

	Solution: The program was broken into two main parts, one part
	checks if given paths are valid or not (isvalid()), and then the
	other part loops through each given paths and copies if it is a
	valid file for copying (copyingfiles()). isvalid() uses isdir()
	and isregular() to determine if a file path is valid for a copy,
	if it is it returns a 1 to copyingfiles() letting it know it can
	copy it, but if it isn't it prints out the reason it's not valid
	and return 1 so copying files doesn't try to copy it.
	copyingfiles() uses buildpath() to know where to put the copy it
	then it loops through the files checking if it is valid with
   isvalid(), if it is then it makes threads to copy the files with
   each iteration of the loop decrementing the files variable which
   will indicate wether or not there are more files to process. at
   the end of the loop it waits for all the files to be processed
   then it ends on success if files were copied and failure if none
   were.

	Data-structure used: A one-dimensional array for the command line
	arguments and a one-dimensional string array.

	Accessing functions for the data structure: Standard C functions
	for accessing arrays and structs.

	Errors handled: The program checks if each file path and it's copy
	path is valid and if the destination path is valid, it also checks
	if the file is already in the place it's trying to be copied too.
	The program also checks if there are errors during the copy
	process to stop it from making a bad file. Lastly, it checks if
	mutex, condition, or thread had a problem with being initalized.

	Limitations: As the file gets bigger the speed in which it gets
	copied gets slower. Can't copy non-regular files.

	Acknowledgment: We wrote the program together but we had a lot of
	help from our classmates throughout the making of it.
*/

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

struct threadinput {
   char *paths[2];
};

int copies = 0, files = 0;
pthread_mutex_t lock;
pthread_cond_t done;

int main(int argc, char *argv[])
{
   int chkdst(char **argv);
   int copyfiles(int argc, char *argv[]);
   void die(char *reason);
   void dispmsg(int s);
   void usage(char *progname);

   files = argc - 2;

   if(pthread_mutex_init(&lock, NULL)) 
      die("MUTEX INITIALIZATION FAILURE");
   if(pthread_cond_init(&done, NULL)) 
      die("CONDITION INITIALIZATION FAILURE");

   if (argc < 3) usage("copy");
   else if (!chkdst(&argv[argc - 1])) die("INVALID DESTINATION");
   else if (!copyfiles(argc, argv)) die("NO COPIES MADE!");

   exit(0);
}

/* Takes progname and print out the correct usage of the program
   to stderr then exits the program with a failure.
   Called by main().
*/
void usage(char *progname)
{
   fprintf(stderr, "./%s file-path1 file-path2 ... dest-dir\n", progname);
   exit(1);
}

/* Takes reason and prints out the reason for the bailout to stderr
   then exits the program with a failure.
   Called by main().
*/
void die(char *reason)
{
   fprintf(stderr, "%s\nPROGRAM ENDED\n", reason);
   exit(1);
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
   fprintf(stderr, "INPUT DESTINATION DOES NOT EXIST\n");
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
      fprintf(stderr, "%-10s: not a regular file\n", path);
      return 0;
   }
   else if (!isregular(path))
   {
      fprintf(stderr, "%-10s: no such file or dir\n", path);
      return 0;
   }
   else if (dst == NULL)
   {
      fprintf(stderr, "%-10s: dest creation error\n", path);
      return 0;
   }
   else if (isregular(dst))
   {
      fprintf(stderr, "%-10s: exist at destination\n", path);
      return 0;
   }

   return 1;
}

/* Creates an array of threadinputs the size of the files then it
   loops throught all the files checking each file if it is a valid
   file to copy (is valid()) if not decrement the files variable and
   go on to the next file else put the srcfile path and dst path to
   the files specific threadinput, a thread is then created for the
   file to be copied (threadcp()). After all the files had been
   processed the program waits for each files thread to be exited if
   no copies are made returns a 0 if not print the message of how
   many copies were made and returns a 1.
   Calls buildpath(), threadcp(), and isvalid().
   Called by main().
*/
int copyfiles(int argc, char *argv[]) 
{
   void buildpath(char *src, char *dst, char **dstpath);
   void *threadcp(void *arg);

   struct threadinput threadinputs[argc - 2];

   int i = 1;
   for (i = 1; i < argc - 1; i++)
   {
      pthread_t thread;
      char *dstpath;
      buildpath(argv[i], argv[argc - 1], &dstpath);

      if (!isvalid(argv[i], dstpath))
      {
         files--;
         continue;
      }
      threadinputs[i - 1].paths[0] = argv[i];
      threadinputs[i - 1].paths[1] = dstpath;

      if (pthread_create(&thread, NULL, threadcp, (void *)
         threadinputs[i-1].paths)) die("PTHREAD CREATION");
  }

   pthread_mutex_lock(&lock);
   while(files > 0) pthread_cond_wait(&done, &lock);
   pthread_mutex_unlock(&lock);

   if (!copies) return 0;
   fprintf(stderr, "\n%d copies copied successfully.\n\n", copies);
   return 1;
}

/* Takes the argument and gets the scr and dst path from it by
   casting it to chars, the file is then copied regardless if it
   fails or not the files variable is decremented but if it
   succeeds copies is incremented and a suceess message is printed
   then the thread is exited, if it fails the thread is just exited.
   Calls makecp().
   Called by copyfiles().  
*/
void *threadcp(void *arg)
{
   int makecp(char *srcpath, char *dstpath);
   char **paths = (char **)arg;
   char *scr = paths[0], *dst = paths[1];

   if (!makecp(scr, dst))
   {
      pthread_mutex_lock(&lock);
      files--;
      pthread_mutex_unlock(&lock);
      pthread_cond_signal(&done);
      pthread_exit(NULL);
   }

   pthread_mutex_lock(&lock);
   files--;
   copies++;
   pthread_mutex_unlock(&lock);
   pthread_cond_signal(&done);

   fprintf(stderr, "%-10s: copied by thread\n", scr);
   pthread_exit(NULL);
}

/* First, checks if scrpath and dstpath are valid to read and write
   to, if not return 0, else loop through till there is nothing
   left to write to dstpath if the loop finishes with no problems
   close all files and return 1.
   Called by threadcp().
*/
int makecp(char *srcpath, char *dstpath)
{
   char bufs[2048];
   int copy, fd, rd;

   if ((fd = open(srcpath, O_RDONLY)) < 0 ||
      (copy = open(dstpath, O_CREAT | O_WRONLY, 0644)) < 0)
   {
      fprintf(stderr, "%-10s: error when accessing\n", srcpath);
       return 0;
   }

   while ((rd = read(fd, bufs, 2048)) > 0)
   {
      if (write(copy, bufs, rd) != rd)
      {
         fprintf(stderr, "%-10s: error when accessing\n", srcpath);
         close(fd);
         close(copy);
         return 0;
      }
   }
   close(fd);
   close(copy);
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
