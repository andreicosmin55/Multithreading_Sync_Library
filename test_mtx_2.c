#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "sinc.h"

Spn mut;

void * abc(void * p) {
  char * msg = (char *) p;
  for (int j = 0; j<100; j++){
    spn_lock(&mut);
        for (size_t i = 0; i < strlen(msg); ++i) {
            printf("%c", msg[i]);
        }
        printf("%d\n",j);
    if(spn_unlock(&mut)){
        printf("\neroare\n");
    }
  }
}

int main() {
    spn_init(&mut);

    pthread_t aa;
    pthread_t bb;
    pthread_t cc;

    pthread_create(&aa, NULL, abc, "A");
    pthread_create(&bb, NULL, abc, "B");
    pthread_create(&cc, NULL, abc, "C");

    pthread_join(aa,NULL);
    pthread_join(bb,NULL);
    pthread_join(cc,NULL);

    spn_destroy(&mut);
    return 0;
}