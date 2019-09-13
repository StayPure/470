/*
Name: Bell, Berman                          Class: CPS470
Section: 22371841                           Assignment: 01
Due: September 12, 2019                     Started: September 6, 2019
Credit: 10 points.

Problem: Write a program that implements a singly linked list to keep
track of events as they occur. An event is defined as a randomly 
generated number from 1 to 53. The list tracks the number 
corresponding to the event and the frequency of occurence up to that 
point. Events that do not exist in the list when the event occurs are
inserted at the end of the list. An event that is already in the list
will have its frequency updated and the node is moved to the front of
the list.

Solution: The solnode list is comprised of user defined structs acting
as a singly linked list. Each solnode holds the numerical value of
each occuring event as well as the frequency of the given event 
occuring. The list is continuously updated and events that occur 
multiple times are appended to the front of the list. This process
continues until the last event is generated.

Data-structure used: A singly linked list created using user defined
struct solnode.

Accessing functions for the data structure: Standard C functions for
accessing lists and structs.

Errors Handled: If the commands in the command line do not match the
required format, an error message is printed to the console
instructing the user on the proper command format.

Limitations: The program only generates events numbered 1 to 53. This
can be changed by altering the value of POOLSZ.

Acknowledgment: Kevin worked on the program extensively for this 
assignment. Nathan helped in debugging and formatting the code as 
well as worked on the documentation. Kevin also helped with the 
content of the documentation.
*/

#include <stdio.h>
#include <stdlib.h>

#define POOLSZ 53
typedef struct solnode {
   int eventnr;
   int eventfq;
   struct solnode *next;
} solnode;

solnode pool[POOLSZ];
solnode *avail;

int main(int argc, char **argv) {
   void displayevents(solnode * list);
   void freelist(solnode * *list);
   void getnode(solnode * *ptr);
   void initpool(void);
   int insertevent(solnode * *list, int event);
   int nextevent(void);
   void usage(char *progname);

   if (argc < 2 || strtol(argv[1], NULL, 10) <= 0) usage("procevent");
   srand(20);

   int evntnum = strtol(argv[1], NULL, 10), i = 0, event;
   initpool();
   solnode *list;
   list = NULL;

   for (i = 0; i < evntnum; i++) {
     event = nextevent();
     if (!insertevent(&list, event)) continue;
   }

   displayevents(list);
   freelist(&list);
   exit(0);
}

/* Design a module that prints out the correct way to use the
   program using the progname to get the name of the program
   ending the program afterwards.
   Takes progname and print out the correct usage of the program
   to stderr then exits the program with a failure.
   Called by main().
*/
void usage(char *progname) {
   fprintf(stderr, "usage: ./%s <event-count>\n", progname);
   exit(1);
}

/* Desgin a module that initializes a pool of available nodes
   Loops POOLSZ amount of times with each one making a node.
   Afterwards sets the available node to the first node in the
   pool array.
   Called by main().
*/
void initpool(void) {
   int i;
   for (i = 0; i < POOLSZ - 1; i++)
     pool[i].next = &pool[i + 1];
   pool[i].next = NULL;
   avail = pool;
}

/* Design a module that obtains a node from the global pool.
   The pointer passed in to the module now points to the first
   available node in the pool. The previously available node
   pointer is then moved to the next available node.
   Called by insertevent().
*/
void getnode(solnode **ptr) {
   *ptr = NULL;
   if (avail != NULL) {
     *ptr = avail;
     avail = avail->next;
   }
}

/* Design a module that returns the node pointed by ptr
   to the available pool.
   Sets the given node to NULL after the operation completes.
   Called by freelist().
*/
void freenode(solnode **tofree) {
   if (*tofree != NULL) {
     (*tofree)->next = avail;
     avail = *tofree;
     *tofree = NULL;
   }
}

/* Design a module that generates an event.
   Returns a randomly generated integer bounded by the
   value of POOLSZ.
   Called by main().
*/
int nextevent(void) { return (rand() % POOLSZ); }

/* Design a module that inserts an event into the linked list at the
   tail.
   Starts by creating four nodes; a current node (crnt), a previous
   node (prev), a previous previous node (pprev), and a newnode. With
   the first three nodes we preform a search (srchevent()) for the
   event if the search worked increment the event and adjust the list
   (adjustlist()) if not check if its the first node to be added if
   so add it using newnode, else it loops till it gets to the end of
   the linkedlist and use newnode to add it to the list, if all fails
   return 0 else return 1.
   Called by main().
*/
int insertevent(solnode **list, int event) {
  void adjustlist(solnode * *list, solnode * pprevious, 
                  solnode * previous, solnode * current);
  void displayevents(solnode * list);
  void srchevent(solnode * list, int xevent, solnode **current,
                 solnode **previous, solnode **pprevious);

   solnode *crnt = NULL, *prev, *pprev, *newnode;
   srchevent(*list, event, &crnt, &prev, &pprev);

   if (crnt) {
     crnt->eventfq++;
     adjustlist(list, pprev, prev, crnt);
     return 1;
   }
   getnode(&newnode);
   crnt = *list;
   if (!crnt) {
     newnode->eventnr = event;
     newnode->eventfq = 1;
     newnode->next = NULL;
     *list = newnode;
     return 1;
   }

   while (crnt) {
     if (crnt->next == NULL) {
       newnode->eventnr = event;
       newnode->eventfq = 1;
       newnode->next = NULL;
       crnt->next = newnode;
       return 1;
     }
     crnt = crnt->next;
   }
   return 0;
}

/* Design a module that searchs through the linked list and finds
   the node with the specified event number.
   Sets the current node to the start of the linked list, and sets
   both previous and previous previous nodes to NULL, then it loops
   through the linked list till it finds the node with the correct
   event number making sure to keep previous node one behind current
   and pprevious two behind current once found it returns.
   Called by insertevent().
*/
void srchevent(solnode *list, int xevent, solnode **current, 
               solnode **previous,solnode **pprevious) {
  void displayevents(solnode * list);

   *current = list;
   *previous = NULL;
   *pprevious = NULL;

   while (*current) {
     if ((*current)->eventnr == xevent) return;
     *pprevious = *previous;
     *previous = *current;
     *current = (*current)->next;
   }
}

/* Design a module that transposes a specified node one posision
   closer to the front of the list.
   First, it checks the current node to see if it is already at the
   top of the list and if not makes the previous node point to the
   node after current, set the current node to point at the previous
   and if there is a previous previous node set it to point at the
   current node otherwise set the current node to the head of the
   list.
   Called by insertevent().
*/
void adjustlist(solnode **list, solnode *pprevious, solnode *previous,
                solnode *current) {
   if (*list == current) return;
   previous->next = current->next;
   current->next = previous;
   if (pprevious) pprevious->next = current;
   else *list = current;
}

/* Design a module that displays the list in the correct format.
   Prints the list contents to stdout as comma separated
   event : frequency pairs.
   Called by main().
*/
void displayevents(solnode *list) {
   solnode *crnt = list;
   while (crnt) {
     if (crnt->eventnr == list->eventnr)
       printf("%d : %d", crnt->eventnr, crnt->eventfq);
     else
       printf(", %d : %d", crnt->eventnr, crnt->eventfq);
     crnt = crnt->next;
   }
   printf("\n");
}

/* Design a module to free every node in the list.
   Returns the storage for list back to the system one node at a time
   by calling freenode(). This process leaves the list empty.
   Called by main().
*/
void freelist(solnode **list) {
   solnode *crnt = *list, *temp;
   while (crnt) {
     temp = crnt;
     crnt = crnt->next;
     freenode(&temp);
   }
   *list = NULL;
}
