#include <stdio.h>
#include <stdlib.h>

#define POOLSZ 53
typedef struct solnode
{
   int eventnr;
   int eventfq;
   struct solnode *next;
} solnode;

solnode pool[POOLSZ];
solnode *avail;

int main(int argc, char **argv)
{
    void displayevents(solnode * list);
    void freelist(solnode * *list);
    void getnode(solnode * *ptr);
    void initpool(void);
    int insertevent(solnode * *list, int event);
    int nextevent(void);
    void usage(char *progname);

    if (argc < 2 || strtol(argv[1], NULL, 10) <= 0)
        usage("procevent");
    srand(20);

    int evntnum = strtol(argv[1], NULL, 10), i = 0, event;
    initpool();
    solnode *list;
    list = NULL;

    for (i = 0; i < evntnum; i++)
    {
        event = nextevent();
        if (!insertevent(&list, event))
            continue;
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
void usage(char *progname)
{
    fprintf(stderr, "usage: ./%s <event-count>\n", progname);
    exit(1);
}

/* Desgin a module that initializes a pool of available nodes
   Loops POOLSZ amount of times with each one making a node.
   Afterwards sets the available node to the first node in the
   pool array.
   Called by main().
*/
void initpool(void)
{
    int i;
    for (i = 0; i < POOLSZ - 1; i++)
        pool[i].next = &pool[i + 1];
    pool[i].next = NULL;
    avail = pool;
}

void getnode(solnode **ptr)
{
    *ptr = NULL;
    if (avail != NULL)
    {
        *ptr = avail;
        avail = avail->next;
    }
}

void freenode(solnode **tofree)
{
    if (*tofree != NULL)
    {
        (*tofree)->next = avail;
        avail = *tofree;
        *tofree = NULL;
    }
}

int nextevent(void)
{
    return (rand() % POOLSZ);
}

int insertevent(solnode **list, int event)
{
    void adjustlist(solnode * *list, solnode * pprevious, solnode * previous, solnode * current);
    void displayevents(solnode * list);
    void srchevent(solnode * list, int xevent, solnode **current, solnode **previous, solnode **pprevious);

    solnode *crnt = NULL, *prev, *pprev, *newnode;
    srchevent(*list, event, &crnt, &prev, &pprev);

    if (crnt)
    {
        crnt->eventfq++; adjustlist(list, pprev, prev, crnt);
        return 1;
    }
    crnt = *list;
    if (!crnt)
    {
        getnode(&newnode); 
        newnode->eventnr = event; newnode->eventfq = 1;
        newnode->next = NULL; *list = newnode;
        return 1;
    }

    while (crnt)
    {
        if (crnt->next == NULL)
        {
            getnode(&newnode);
            newnode->eventnr = event; newnode->eventfq = 1;
            newnode->next = NULL; crnt->next = newnode;
            return 1;
        }
        crnt = crnt->next;
    }
    return 0;
}

void srchevent(solnode *list, int xevent, solnode **current, solnode **previous, solnode **pprevious)
{
    void displayevents(solnode * list);

    *current = list; *previous = NULL; *pprevious = NULL;

    while (*current)
    {
        if ((*current)->eventnr == xevent) return;
        *pprevious = *previous; *previous = *current;
        *current = (*current)->next;
    }
    return;
}

/* I'll fix this and change it
*/
void adjustlist(solnode **list, solnode *pprevious, solnode *previous, solnode *current)
{
    if (*list == current) return;
    previous->next = current->next; current->next = *list;
    *list = current;
    if (pprevious) pprevious->next = previous;
    return;
}

void displayevents(solnode *list)
{
    solnode *crnt = list;
    while (crnt)
    {
        if (crnt->eventnr == list->eventnr) 
           printf("%d : %d", crnt->eventnr, crnt->eventfq);
        else   
           printf(", %d : %d", crnt->eventnr, crnt->eventfq);
        crnt = crnt->next;
    }
    printf("\n");
    return;
}

void freelist(solnode **list)
{
    solnode *crnt = *list, *temp;
    while (crnt)
    {
        temp = crnt;
        crnt = crnt->next;
        freenode(&temp);
    }
    *list = NULL;
    return;
}
