#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<pthread.h>
#include"kumud_semaphore.h"

/*Data Structure for shared Ring Buffer*/
struct Ring_Buffer_Area{
    int *buffer;
    int head;
    int tail;
}items;

struct Ring_Buffer_Area *rbarea;

/*Initialization of pthread_mutex_t pmutex and pthread_cond_t condition*/
pthread_mutex_t pmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition=PTHREAD_COND_INITIALIZER;

/*Global variables declaration*/
Semaphore mutex,emptyCount,fullCount;
int num_producers,num_consumers,num_items,buffer_size,max_sleep_secs;
int *total_items_produced, *total_items_consumed;

/*Producer thread routine*/
void *do_produce(void *pid)
{
  int ptid;
  int next_produced;

  ptid= *((int*)pid);

  while(1)
   {
    
    /*checking if total items produced by thread is equal to number of items to be produced*/
    if(total_items_produced[ptid] == num_items)
      break;

    next_produced = rand() % 20;/*generating random integer between 0 and 19 */
    sleep((rand() % max_sleep_secs)+1);/*Producer thread pretending to produce by sleeping between 1 and maximum sleeping secs*/

    /*entry section*/
    P(&emptyCount);/*producer waits until the buffer is empty*/
    P(&mutex); /*acquire lock*/
   
    /*critical section*/
    rbarea->buffer[rbarea->head]=next_produced;
    printf("Item produced =%d\n",next_produced);
    rbarea->head=(rbarea->head + 1)% buffer_size;
   
    total_items_produced[ptid]++; /*keeping track of total items produced*/

    /*exit section*/
    V(&mutex); /*release lock*/
    V(&fullCount);
    
  }
   
}

/*Consumer thread routine*/
void *do_consume(void *cid)
{
    int ctid;
    int next_consumed;

    ctid= *((int *)cid);
    while(1)
    { 
      /*checking if total items consumed is equal to number of items to be consumed*/
      if(total_items_consumed[ctid] == num_items)
      break;

      /*entry section*/
      P(&fullCount); /*wait until the buffer is full*/
      P(&mutex);/*acquire lock*/
      
      /*critical section*/
      next_consumed = rbarea->buffer[rbarea->tail];
      printf("Item consumed = %d\n",next_consumed);
      rbarea->tail=(rbarea->tail + 1)% buffer_size;
      
      total_items_consumed[ctid]++; /*keeping track of total items consumed*/
      
      /*exit section*/
      V(&mutex);
      V(&emptyCount); 
      
      sleep((rand() % max_sleep_secs) + 1); 
    }
     
     
}

void total_count_check()
{
    int i;
    int total_produce_count=0; /*variable keeping track of total number of items produced by all producer threads*/
    int total_consume_count=0; /*variable keeping track of total number of items consumed by all consumer threads*/

    for(i=0; i<num_producers ;i++)
    {
      total_produce_count+=total_items_produced[i];
    }

    for(i=0; i<num_consumers; i++)
    {
      total_consume_count+=total_items_consumed[i];
    }

    printf("Total items produced = %d\n",total_produce_count);
    printf("Total items consumed = %d\n", total_consume_count);

    if(total_produce_count == total_consume_count)
    {
      printf("Great!! Your program is successful :) Hurray \n");
    }

    else
    {
      printf("Oops!! Sorry your program has a bug :( Please correct it\n");
    }
}
int main(int argc, char *argv[])
{
/*Parallel program using own version of semaphore(kumud_semaphore.h) and creating M producer threads and N Consumer threads*/

    /*Variables Declaration*/
    int i;
    pthread_t *producer_threads,*consumer_threads;
    pthread_attr_t *pattrs,*cattrs;
    int *producer_tids,*consumer_tids;
    void *retval;

    if(argc!=6)
    {
      fprintf(stderr,"Usage:main[# of producers], [# of consumers], [max_sleep time], [# of items to be produced],[buffer size]\n");
      exit(1);
    }

    /*Getting respective arguments from command line and changing them into integers*/
    num_producers = atoi(argv[1]);
    num_consumers = atoi(argv[2]);
    max_sleep_secs = atoi(argv[3]);
    num_items = atoi(argv[4]);
    buffer_size = atoi(argv[5]);

    /*Error checking*/
    if(num_producers < 2)
    {
      fprintf(stderr, "argument 2, that is number of producer threads must be greater than 1 because it is multi thread producer consumer program\n");
      exit(1);
    }

    if(num_consumers < 2)
    {
      fprintf(stderr, "argument 3, that is number of consumer threads  must be greater than 1 because it is multi threaded producer consumer program\n");
      exit(1);
    }

    if(max_sleep_secs < 0)
    {
      fprintf(stderr, "argument 4, that is maximum sleep second  must be greater than 0\n");
      exit(1);
    }

    if(num_items < 0)
    {
      fprintf(stderr, "argument 5, that is number of items to be produced must be greater than 0\n");
      exit(1);
    }

    if(buffer_size < 0)
    {
      fprintf(stderr, "argument 6, that is buffer size  must be greater than 0\n");
      exit(1);
    }

    /*Allocating memory*/
    rbarea=malloc(sizeof(struct Ring_Buffer_Area));
    rbarea->buffer=(int*)malloc(sizeof(int)*buffer_size);

    total_items_produced=(int*)malloc(sizeof(int)*num_producers);
    total_items_consumed=(int*)malloc(sizeof(int)*num_consumers); 
  
    producer_threads = (pthread_t*)malloc(sizeof(pthread_t)*num_producers);
    consumer_threads = (pthread_t*)malloc(sizeof(pthread_t)*num_consumers);

    pattrs = (pthread_attr_t*)malloc(sizeof(pthread_attr_t)*num_producers);
    cattrs = (pthread_attr_t*)malloc(sizeof(pthread_attr_t)*num_consumers);

    producer_tids = (int *)malloc(sizeof(int)*num_producers);
    consumer_tids = (int *)malloc(sizeof(int)*num_consumers);


    /*initializing semaphores mutex, emptyCount, fullCount values */
    semaphore_init(&mutex,1);
    semaphore_init(&emptyCount,buffer_size);
    semaphore_init(&fullCount, 0); 
    
    /*Creating the producer threads */
    for(i=0; i<num_producers; i++)
    {
      if(pthread_attr_init(pattrs+i))
      {
        perror("attr_init()");
      }
      producer_tids[i]=i;
      if(pthread_create(producer_threads+i, pattrs+i,do_produce,producer_tids+i) !=0)
      {
        perror("pthread_create()");
        exit(1);
      }
      else
      {
        printf("Created producer thread T%d\n",i);
      }
    }

    /*Creating the consumer threads */
    for(i=0;i<num_consumers;i++)
    {
      if(pthread_attr_init(cattrs+i))
      {
        perror("attr_init()");
      }
      consumer_tids[i]=i;
      if(pthread_create(consumer_threads+i,cattrs+i,do_consume,consumer_tids+i) !=0)
      {
        perror("pthread_create()");
        exit(1);
      }
      else
      {
        printf("Created consumer thread T%d\n",i);
      }
    }


     /*Joining threads*/
     for(i=0; i<num_producers;i++)
     {
       pthread_join(producer_threads[i],&retval);
       printf("Producer thread T%d is finished\n",i);
     }

     for(i=0; i<num_consumers;i++)
     {
       pthread_join(consumer_threads[i],&retval);
       printf("Consumer thread T%d is finished\n",i);
     }       
     
     /*comparing total items produced count and total items consumed count*/
     total_count_check();

     /*deallocating the memory previously allocated by malloc*/ 
     free(total_items_produced);
     free(total_items_consumed);
     free(producer_threads);
     free(consumer_threads);
     free(pattrs);
     free(cattrs);
     free(producer_tids);
     free(consumer_tids);
     free(rbarea->buffer);
     free(rbarea);

     return 0;

}
      
        
    

 
