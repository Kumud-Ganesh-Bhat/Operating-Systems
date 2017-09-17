#ifndef KUMUD_SEMAPHORE_H_ /*Checks whether the given token has been #defined earlier in the file or in an included file. If not, it includes the code between it and the closing #endif statement*/
#define KUMUD_SEMAPHORE_H_
#include<pthread.h>

/*Implementing own version of semaphore using pthread mutex and pthread condition variable*/

/*Structure defining a semaphore*/
struct semaphore
{
    int value;
    pthread_mutex_t pmutex;
    pthread_cond_t condition;
};

typedef struct semaphore Semaphore;

void P(Semaphore *s); /*wait operation*/
void V(Semaphore *s); /*signal operation*/

#endif

