#include<pthread.h>
#include<stdio.h>
#include<semaphore.h>
#include<unistd.h> 
#include "sinc.h"

int nr=1;
Semafor s;

void* fct1()
{
    s_wait(&s);
    printf("T1 citeste %d\n",nr);
    nr++;
    printf("T1 modifica %d\n",nr);    
    sleep(1);
    s_post(&s);
}
void* fct2()
{
    s_wait(&s);
    printf("T2 citeste %d\n",nr);
    nr--;
    printf("T2 modifica %d\n",nr);
    s_post(&s);
}

 int main()
 {
    _s_init(&s,1);

    pthread_t t1, t2;
    pthread_create(&t1, NULL, fct1, NULL);
    pthread_create(&t2, NULL, fct2, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2,NULL);

    printf("Final nr = %d\n",nr);

    s_destroy(&s);
 }