#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sinc.h"

pthread_t tid[2];
int counter;
Spn loc;
  
void* trythis(void* arg)
{
    spn_lock(&loc);

    counter += 1;

    printf("Job %d has started\n", counter);
  
    printf("Job %d has finished\n\n", counter);
  
    spn_unlock(&loc);
  
    return NULL;
}
  
int main(void)
{
    int i = 0;
    int error;
  
    spn_init(&loc);
  
    while (i < 100) {
        error = pthread_create(&(tid[i]),
                               NULL,
                               &trythis, NULL);
        if (error != 0)
            printf("\nThread can't be created :[%s]",
                   strerror(error));
        i++;
    }
  
    for(int j = 0; j< 100;j++)
        pthread_join(tid[j], NULL);

    spn_destroy(&loc);
  
    return 0;
}