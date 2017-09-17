#include<stdio.h> /* header file containing info about standard input, output */
#include<sys/types.h> /*header file containg definitions of number of data types used in system calls */
#include<sys/ipc.h> /*header file contains definitons for interprocess commn access structure */
#include<sys/shm.h> /* header file that describes the structures that are used by the subroutines that perform shared memory operations */
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<time.h>

#define BUFFER_SIZE 40000000 /*Symbolic Constant representing Ring Buffer Size */

/*Data Structure for Ring Buffer */
typedef struct Ring_Buffer_Area {
  int buffer[BUFFER_SIZE];
  int head;
  int tail;
}item;


/*Function Prototypes*/
void do_parent_process(struct Ring_Buffer_Area*,int); /*Producer or Driver process*/
void do_child_process(struct Ring_Buffer_Area*);  /*Consumer or Application process*/
struct Ring_Buffer_Area* create_global_ring_buffer_area(size_t poolsize_per_core, int *shmid); /*Function returning pointer to shared ring buffer area */

int main(int argc, char *argv[])
{
/* System V Shared Memory program for interprocess communication */

    /*Variables declaration*/
    struct Ring_Buffer_Area* g_rbarea; /*pointer to the shared memory segment */
    int shmid; /* identifier for the shared memory segment */
    
    
    g_rbarea=create_global_ring_buffer_area(sizeof(*g_rbarea),&shmid);
   

    if(fork() == 0)
    {
      /* Am child process-Consumer */
      do_child_process(g_rbarea);
    }

    else
    {
      /*Am parent process-Producer */
      do_parent_process(g_rbarea,shmid);
    }

return 0;
}

struct Ring_Buffer_Area* create_global_ring_buffer_area(size_t size, int *shmid)
{
    /*Function to get pointer of shared memory data segment*/

    /*Variables declaration and initialization*/
    key_t key = 2015;
    int shmflag=0644 | IPC_CREAT ; /*variable being set to the permissions of segment*/
    void *shm; /*void pointer to the shared memory segment returned by shmat call */
    struct Ring_Buffer_Area* g_rbarea;

    /*Creating the segment and connecting*/
    if( (*shmid=shmget(key,size,shmflag)) < 0)
    {
      perror("shmget");
      exit(1);
    }

    /*Attach operation to get pointer to the segment */
    if ( (shm=shmat(*shmid,(void *)0,shmflag)) == (void *) -1)
    {
      perror("shmat");
      exit(1);
    }

    /*Treating void pointer as struct Ring_Buffer_Area pointer*/
    g_rbarea = (struct Ring_Buffer_Area*)shm;

return g_rbarea;

}

void do_parent_process(struct Ring_Buffer_Area* g_rbarea, int shmid)
{
    /*Variables declarion*/
    
    int status,next_produced;
    int parent_sum=0;
    clock_t start_time,end_time;   
   
    printf("Am Parent(Producer) process: My id is %d\n",getpid());
   

    start_time=clock(); /*Starting Time*/
    
    /*Generate an random integer between 0 and 19 in next_produced until the buffer is not full*/
      
      while(((g_rbarea->head + 1) % BUFFER_SIZE) != g_rbarea->tail)
      {
        next_produced=rand() % 20;
        parent_sum+=next_produced;
        g_rbarea->buffer[g_rbarea->head]=next_produced;
        g_rbarea->head = (g_rbarea->head + 1) % BUFFER_SIZE;
        
      }
     
    end_time=clock(); /*finishing time*/

    printf("Sum of integer messages produced = %d\n", parent_sum);
    printf("Total time taken by parent i.e., driver process = %f\n", ((double)(end_time - start_time))/CLOCKS_PER_SEC );

    wait(&status);
    /* Detaching the shared memory segment */
    if(shmdt(g_rbarea) == -1)
    {
      printf("Parent:Cannot detach memory segment\n");
      exit(1);
    }

    /* Removing the shared memory segment */
    if(shmctl(shmid, IPC_RMID, 0))
    {
       perror("shmctl");
       exit(1);
    }
    
    printf("Parent removed memory segment\n");

}

void do_child_process(struct Ring_Buffer_Area* g_rbarea)
{
    /*Variables declaration */
    int child_sum=0;
    int next_consumed;
    clock_t start,end;

    sleep(10);

    start=clock(); /*Starting time*/

 
    while(g_rbarea->head != g_rbarea->tail)
    {
        /*Until buffer is empty*/
        next_consumed = g_rbarea->buffer[g_rbarea->tail];
        child_sum+=next_consumed;
        g_rbarea->tail = (g_rbarea->tail + 1) % BUFFER_SIZE;
    
    }
     
    end=clock(); /*Ending time*/
    
    printf("Sum of integer messages consumed = %d\n", child_sum);
    printf("Total time taken by child i.e., application process =%f\n", ((double)(end-start))/CLOCKS_PER_SEC);

    /* Detaching from memory segment */
    if(shmdt(g_rbarea) == -1)
    {
       printf("Child: Cannot detach memory segment\n");
       exit(1);
    }

    printf("Child process is exiting by calling exit(111)\n");
    exit(111);   
}
