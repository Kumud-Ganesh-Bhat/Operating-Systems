#include<stdio.h> /*header file containing info about standard input,output*/
#include<sys/types.h> /* header file containing definitions of number of data types used in system calls */
#include<sys/ipc.h> /*header file containing definitions for interprocess commn access structure */
#include<sys/shm.h> /*header file that describes the structures that are used by the subroutines that perform shared memory operations */
#include<fcntl.h> /*header file that defines 0_* constants */
#include<sys/stat.h> /*header file that defines mode constants*/
#include<sys/mman.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<time.h>

#define BUFFER_SIZE 512
#define NAME "O_EXCL"

/*Data Structure for Ring Buffer*/
typedef struct Ring_Buffer_Area{
    int buffer[BUFFER_SIZE];
    int head;
    int tail;
}item;

/*Function Prototypes*/
void do_parent_process(struct Ring_Buffer_Area*); /*Producer or Driver process */
void do_child_process(struct Ring_Buffer_Area*); /*Consumer or Application process */
struct Ring_Buffer_Area* create_global_ring_buffer_area(size_t poolsize_per_core, int *shm_fd); /*Function returning pointer to shared memory segment*/


int main(int argc,char *argv[])
{
/* POSIX API program for Interprocess Communication*/

    /*Variables declaration*/
    struct Ring_Buffer_Area* g_rbarea; /*pointer to the shared memory object*/
    int shm_fd; /*shared memory file descriptor*/
    
    g_rbarea=create_global_ring_buffer_area(sizeof(*g_rbarea),&shm_fd);

    if(fork()==0)
    {
      /*Am child process-Consumer*/
      do_child_process(g_rbarea);
    }

   else
   {
     /*Am parent process-Producer*/
     do_parent_process(g_rbarea);
   }

return 0;

}

struct Ring_Buffer_Area* create_global_ring_buffer_area(size_t size, int *shm_fd)
{
/*Function to get pointer of shared memory object*/

   /*Variables declaration and initialization*/ 
   void* ptr;
   struct Ring_Buffer_Area* g_rbarea;
  
   /*Process creating shared memory object*/
   if((*shm_fd=shm_open(NAME,O_CREAT | O_RDWR , 0666))== -1)
   {
     perror("shm_open");
     exit(1);
   }
   
   /*configure the size of the shared memory object*/
   ftruncate(*shm_fd,BUFFER_SIZE);

   /*memory map the shared memory object*/
   if((ptr=mmap(NULL,BUFFER_SIZE,PROT_WRITE | PROT_READ,MAP_SHARED,*shm_fd,0)) == MAP_FAILED)
   {
      perror("mmap");
      exit(1);
   }

   g_rbarea = (struct Ring_Buffer_Area *)ptr;

   return g_rbarea;
}

void do_parent_process(struct Ring_Buffer_Area* g_rbarea)
{
    /*Variables declaration*/
    int status, next_produced;
    int parent_sum=0;
    clock_t start_time,end_time;

    printf("Am parent(producer) process: my id is %d\n", getpid());

    start_time=clock(); /*Starting Time*/
    
    /*Generate an random integer until the buffer is not full */
    while(((g_rbarea->head + 1) % BUFFER_SIZE) != g_rbarea->tail) 
    {
      next_produced=rand()%20;
      parent_sum+=next_produced;
      g_rbarea->buffer[g_rbarea->head]=next_produced;
      g_rbarea->head = (g_rbarea->head + 1)  % BUFFER_SIZE;
    }

    end_time=clock();/*Finishing time*/

    printf("Sum of integer messages produced = %d\n", parent_sum);
    printf("Total time taken by parent/driver process = %f\n", ((double)(end_time - start_time))/CLOCKS_PER_SEC);

    wait(&status);
}

void do_child_process(struct Ring_Buffer_Area* g_rbarea)
{
   /*Variables declaration*/
   int child_sum=0;
   int next_consumed;
   clock_t start,end;  

   sleep(10); 
   
   start=clock(); /*Starting time*/  
   

   while(g_rbarea->head !=g_rbarea->tail)
   {
     /*Until buffer is not empty*/
     next_consumed=g_rbarea->buffer[g_rbarea->tail];
     child_sum+=next_consumed;
     g_rbarea->tail=(g_rbarea->tail + 1) % BUFFER_SIZE;
   }

   end=clock(); /*End Time*/

   printf("Sum of integer messages consumed = %d\n", child_sum);
   printf("Total time taken by child/application process = %f\n", ((double)(end-start))/CLOCKS_PER_SEC);

   /*Removing the shared memory segment after the consumer has accessed it*/
   if(shm_unlink(NAME) == -1)
   {
     printf("Child i.e., Application process cannot remove memory segment\n");
     exit(1);
   }

   printf("Child process is exiting by calling exit(111)\n");
   exit(111);
}
   

    

