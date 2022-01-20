#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include "sinc.h"

#define NrThreads 10  

Bariera bar;


void* thread_routine(void* arg){
    int val = *(int*)arg;

    printf("threadul %d a ajuns la bariera\n", val);
    bar_point(&bar);
    printf("threadul %d a trecut de bariera\n", val);

    return NULL;
}

int main(){
    pthread_t thread[NrThreads];
    int arg[NrThreads];
    int ThrBarrier = 5;

    if(NrThreads % ThrBarrier){
        printf("Programul ar rula la infinit; ABORTED");
        return 0;
    }

    bar_init(&bar, ThrBarrier, NrThreads);

    for(int i = 0; i < NrThreads; i++){
        arg[i] = i+1;
        if(pthread_create(&thread[i], NULL, thread_routine, &arg[i])){
            perror("pthread_create");
        }
    }
    
    for(int i = 0; i < NrThreads; i++){
        if(pthread_join(thread[i], NULL)){
            perror("pthread_join");
        }
    }

    sem_destroy(&bar);
}