#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "sinc.h"

int nr = 0;
Rwlock rwl;

void* readt(){
    lock_for_reading(&rwl);
    printf("READ %d,counter %d\n", nr, rwl.counter);
    unlock_for_reading(&rwl);

    return NULL;
}

void* writet(){
    lock_for_writing(&rwl);
    nr ++;
    printf("WRITE %d,counter %d\n", nr, rwl.counter);
    unlock_for_writing(&rwl);

    return NULL;
}

#define NR_THREADS 30000    //maxim 32000 ca moare de la segmentation fault
int main()
{
    pthread_t threads[NR_THREADS];
    rwl_init(&rwl);

    for(int i = 0; i < NR_THREADS; i++){
        if(i%2)
            pthread_create(&threads[i], NULL, readt, NULL);
        else
            pthread_create(&threads[i], NULL, writet, NULL);
    }

    for(int i = 0; i < NR_THREADS; i++){
        pthread_join(threads[i],NULL);
    }
    printf("\nnr:%d  counter: %d",nr,rwl.counter);
    return 0;
}