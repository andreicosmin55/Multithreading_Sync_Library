#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "sinc.h"

#define MAX_RESOURCES 600

int available_resources = MAX_RESOURCES;

Spn mutex;

int decrease_count(int count)
{
    if (available_resources >= count){
        available_resources -= count;
        printf("got %d, %d remaining\n", count, available_resources);
        if(spn_unlock(&mutex)){
            perror("mutex_unlock");
        }
        return 0;
    }
    
    if(spn_unlock(&mutex)){
        perror("mutex_unlock");
    }
    return -1;

}

int increase_count(int count)
{
    available_resources += count;
    printf("released %d, %d remaining\n", count, available_resources);
    if(spn_unlock(&mutex)){
        perror("mutex_unlock");
    }
    return 0;
}

void* thread_routine(void *arg){
    int val = *(int*)arg;

    spn_lock(&mutex);
    while(decrease_count(val)){
        spn_lock(&mutex);
    }

    spn_lock(&mutex);

    increase_count(val);

    return NULL;
}

int main(){
    pthread_t thread[500];
    int arg[500];

    spn_init(&mutex);

    for(int i = 0; i < 500; i++){
        arg[i] = i+1;
        if(pthread_create(&thread[i], NULL, thread_routine, &arg[i])){
            return errno;
        }
    }

    for(int i = 0; i < 500; i++){
        if(pthread_join(thread[i], NULL)){
            return errno;
        }
    }

    spn_destroy(&mutex);
}