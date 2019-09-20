/*
	Name: Bell, Berman						   Class: CPS 470
   Section: 22371841                      Assignment: 02
   Due: September 19, 2019                Started: September 18, 2019 
   Credit: 10 points.

	Problem: Write a program that copies the given files to the 
	specified destination directory using file system call interface
	(open(), close(), read(), write(), stat() etc.). The program 
	should only handle regular files coming from the specified source
	paths. The file paths will be entered through command line
	arguments. The program should give the  usage of the program when
	the incorrect number of arguments are given (< 2), end before
	anything if the destination is invalid, and only copy files that
	are regular files and if they aren't stop the copy and give the
	reason for not doing the copy. Throughout the copying operation
	the program should indicate that it is still working.

	Solution: The program was broken into two main parts, one part
	checks if given paths are valid or not (isvalid()), and then the
	other part loops through each given paths and copies if it is a
	valid file for copying (copyingfiles()). isvalid() uses isdir()
	and isregular() to determine if a file path is valid for a copy,
	if it is it returns a 1 to copyingfiles() letting it know it can
	copy it, but if it isn't it prints out the reason it's not valid
	and return 1 so copying files doesn't try to copy it.
	copyingfiles() uses buildpath() to know where to put the copy it
	then lets isvalid() check the file if it is valid, it sends the
	original's path and the copy's path to makecp() for it to copy
	which also can cancel if there is any errors, once copied it loops
	through the rest of the other files while it operates an alarm 
	goes off every second to show that it hasn't stop working. This
	ends once copyingfiles() have gone through every file and it
	returns a 0 if no copies were made ending the program in an
	EXIT_FAILURE and 1 (EXIT_SUCCESS) otherwise.

	Data-structure used: A one-dimensional array for the command line
	arguments.

	Accessing functions for the data structure: Standard C functions
	for accessing arrays and structs.

	Errors handled: The program checks if each file path and it's copy
	path is valid and if the destination path is valid, it also checks
	if the file is already in the place it's trying to be copied too.
	The program also checks if there are errors during the copy
	process to stop it from making a bad file.

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
#include <unistd.h>

int main(int argc, char *argv[]) {
   int chkdst(char **argv);
   int copyfiles(int argc, char *argv[]);
   void die(char *reason);
   void dispmsg(int s);
   void usage(char *progname);

   signal(SIGALRM, dispmsg);
   alarm(1);

   if (argc < 3) {
      char str[] = "copy", *strptr = str;
      usage(strptr);
   } else if (!chkdst(&argv[argc - 1])) {
      char str[] = "INVALID DESTINATION", *strptr = str;
      die(strptr);
   } else if (!copyfiles(argc, argv)) {
      char str[] = "NO COPIES MADE!", *strptr = str;
      die(strptr);
   }

   exit(0);
}

/* Design a module that handles signals from alarms
   When a signal arrives it checks if the siginal is zero if its not
	it dispays the message and resets the alarm.
 */
void dispmsg(int s) {
   if (s) {
      printf("copy: still working...\n");
      alarm(1);
   }
}

/* Design a module that prints out the correct way to use the
   program using the progname to get the name of the program
   ending the program afterward.
   Takes progname and print out the correct usage of the program
   to stderr then exits the program with a failure.
   Called by main().
*/
void usage(char *progname) {
   fprintf(stderr, "./%s file-path1 file-path2 ... dest-dir\n", 
  			  progname);
   exit(1);
}

/* Design a module that kills the program when for a given reason
   printing the reason for the bailout before exiting.
   Takes reason and prints out the reason for the bailout to stderr
   then exits the program with a failure.
   Called by main().
*/
void die(char *reason) {
   fprintf(stderr, "%s\nPROGRAM ENDED\n", reason);
   exit(1);
}

/* Design a module that takes in a single arg, char **argv,
   that checks the sanity of the last arg passed to main()
   returns a 1 on success or 0 otherwise. A meaningful/usage
   error message will be printed out on failure.
   Takes a single pointer, *argv, and passes it to isdir()
   to check if the directory exists. If isdir returns a 1 a
   1 is returned from this module. Otherwise, an error message
   is printed and a 0 is returned.
   Calls isdir().
   Called by main().
*/
int chkdst(char **argv) {
   int isdir(char *path);

   if (isdir(*argv)) return 1;
   fprintf(stderr, "INPUT DESTINATION DOES NOT EXIST\n");
   return 0;
}

/* Design a module that takes in a single arg, char *path,
   checks whether it is a directory or not, and returns an
   int.
   Uses the Stat struct to construct a struct, sbuf,
   and uses stat() to obtain information from the file and
   write it to sbuf. Uses S_ISDIR() on sbuf.st_mode to see
   the mode of the file. A 1 is returned if the file is a
   directory otherwise a 0 is returned.
   Called by isvalid().
*/
int isdir(char *path) {
   struct stat sbuf;
   if (stat(path, &sbuf)) return 0;
   return S_ISDIR(sbuf.st_mode);
}

/* Design a module that takes in a single arg, char *path,
   and returns a 1 if it is a regular file or a 0 otherwise.
   Uses the Stat struct to construct a struct, sbuf,
   and uses stat() to obtain information from the file and
   write it to sbuf. Uses S_ISREG on sbuf.st_mode to see if
   the file is regular. A 1 is returned if the S_ISREG is true
   otherwise a 0 is returned.
   Called by isvalid().
*/
int isregular(char *path) {
  struct stat sbuf;
   if (stat(path, &sbuf)) return 0;
   return S_ISREG(sbuf.st_mode);
}

/* Design a module that checks the validity of a
   source path and destination path, return 1 if it is valid
   0 otherwise
   Checks if the source path is a directory first, then if its
   a regular file return 0 if it is dir and if it isn't a regular
   file, then checks if the destionation path was created or if
   the file exist at the destination if either return 0, if none
   of these return 1.
   Calls isdir() and isregular().
   Called by copyfiles().
*/
int isvalid(char *path, char *dst) {
   if (isdir(path)) {
      fprintf(stderr, "copy: %s is not a regular file\n", path);
      return 0;
   } else if (!isregular(path)) {
      fprintf(stderr, "copy: %s does not exist\n", path);
      return 0;
   } else if (dst == NULL) {
      fprintf(stderr, "copy: %s had a dest creation error\n", path);
      return 0;
   } else if (isregular(dst)) {
      fprintf(stderr, "copy: %s exist at destination\n", path);
      return 0;
   }

   return 1;
}

/* Design a module that takes the arguments and copies the files
   to the designated destination directory if it is a valid file.
   if the copies couldn't be made return 0, otherwise return 1.
   Loops through all of the file paths given checking each if it's
   path or destination path is valid or not, if it isn't skip it
   otherwise make a copy, if no copies are made after the loop
   return 1 if some were made print the number of copies to the
   stderr and return 0.
   Calls buildpath, makecp(), and isvalid().
   Called by main().
*/
int copyfiles(int argc, char *argv[]) {
   void buildpath(char *src, char *dst, char **dstpath);
   int makecp(char *srcpath, char *dstpath);

   int i = 1, copies = 0;
   for (i = 1; i < argc - 1; i++) {
      char *dstpath;
      buildpath(argv[i], argv[argc - 1], &dstpath);

      if (!isvalid(argv[i], dstpath)) continue;

      if (!makecp(argv[i], dstpath)) continue;

      copies++;
   }

   if (!copies) return 0;

   fprintf(stderr, "copy: all done\n\n");
   return 1;
}

/* Design a module that takes a source path and copies that file
   to the destination path given, returns 1 on sucessful copy and
   0 otherwise.
   First, checks if scrpath and dstpath are valid to read and write
   to, if not return 0, else loop through till there is nothing
   left to write to dstpath if the loop finishes with no problems
   close all files and return 1.
   Called by copyfiles().
*/
int makecp(char *srcpath, char *dstpath) {
   char bufs[2048];
   int copy, fd, rd;

   if ((fd = open(srcpath, O_RDONLY)) < 0 ||
      (copy = open(dstpath, O_CREAT | O_WRONLY, 0644)) < 0) {
      fprintf(stderr, "copy: error when accessing %s\n", srcpath);
      return 0;
   }

  while ((rd = read(fd, bufs, 2048)) > 0) {
      if (write(copy, bufs, rd) != rd) {
         fprintf(stderr, "copy: error when accessing %s\n", srcpath);
         close(fd);
         close(copy);
         return 0;
      }
   }
   close(fd);
   close(copy);
   return 1;
}

/* Builds destination-path using strrchr() function from
   library, dstpath is null on error and defined otherwise.
   Caller should check the result.
*/
void buildpath(char *src, char *dst, char **dstpath) {
   char *ptr;
   int n;
   ptr = strrchr(src, '/');

   if (ptr)
      n = strlen(dst) + strlen(ptr) + 2;
   else
      n = strlen(dst) + strlen(src) + 2;

   *dstpath = malloc(n);

   if (!dstpath) return;

   if (ptr) {
      strcpy(*dstpath, dst);
      strcat(*dstpath, ptr);
   } else {
      strcpy(*dstpath, dst);
      strcat(*dstpath, "/");
      strcat(*dstpath, src);
   }
}
