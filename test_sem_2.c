#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include "sinc.h"

#define nMAX 315153

Semafor s;
int nr = 0;

void* fct(void * a){
    int i,aux;
    //int* q = malloc(sizeof(int));
    for(i = 0; i < nMAX; i++){
        s_wait(&s);
        // s_getvalue(q, &s);
        // printf("%d\n", *q);
        aux = nr; 
        aux = aux+1;
        nr = aux; 
        s_post(&s);
    }
}

int main(){    
    _s_init(&s, 1);

    pthread_t t1, t2;
    pthread_create(&t1, NULL, fct, NULL);
    pthread_create(&t2, NULL, fct, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    if (nr < 2 * nMAX) 
        printf("\n FAIL! nr = %d, trebuia = %d\n", nr, 2*nMAX);
    else
        printf("\n OK! nr = [%d]\n", nr);

    s_destroy(&s);
}
