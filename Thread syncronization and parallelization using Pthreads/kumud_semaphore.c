#include "kumud_semaphore.h"

/*initialising semaphore value */
void semaphore_init(Semaphore *s, int value)
{
    s->value=value;
}


/* wait (P) operation for semaphore */
void P(Semaphore *s)
{
    pthread_mutex_lock(&s->pmutex); /*acquire the mutex lock and this should be locked before calling cond_wait()*/
    while(s->value <=0)
    {
        pthread_cond_wait(&s->condition, &s->pmutex); /*suspend the thread that is calling wait*/
    }
    s->value--;
    pthread_mutex_unlock(&s->pmutex); /*release the mutex lock*/
}

/* signal(V) operation for semaphore */
void V(Semaphore *s)
{
    pthread_mutex_lock(&s->pmutex);/*acquire the mutex lock*/
    s->value++;
    /*pthread_cond_broadcast(&s->condition);*/ /* signal all threads. Needed in certain situations. This may lead to spurious wakeup */
    pthread_cond_signal(&s->condition); /*signal one thread and wake it up*/
    pthread_mutex_unlock(&s->pmutex); /*release mutex lock*/
}

