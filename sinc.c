#include "sinc.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>
#include <sys/syscall.h>
#include <unistd.h>

//                                   SPINLOCK cu ownership
//  ===================================================================================

void spn_init(Spn* Mut){
    Mut->owner = 0;
}

void spn_lock(Spn* Mut){
    while (!__sync_bool_compare_and_swap(&Mut->owner, 0, syscall(__NR_gettid))){};
}   

int spn_unlock(Spn* Mut) {
    if (Mut->owner == 0){
        atomic_store(&Mut->owner, 0);
        return -1;  //EROARE UNLOCK LA CEVA UNLOCKED
    }
    //while (!__sync_bool_compare_and_swap(&Mut->owner, syscall(__NR_gettid), 0)){};
    if (Mut->owner != syscall(__NR_gettid)){
        atomic_store(&Mut->owner, 0);
        return -2;  //EROARE OWNERSHIP
    }
    atomic_store(&Mut->owner, 0);
    return 0;
}

int spn_destroy(Spn* Mut){
    if (Mut->owner){
        printf("eroare in destroyer");
        return -1;
    }
    return 0;
}



//                                  SEMAFOR BINAR
//  ====================================================================================

void sem_binar_init(Sem_binar* sb){
    atomic_flag_clear(&sb->flg);
}

void lock_sb(Sem_binar* sb){
    while (atomic_flag_test_and_set(&sb->flg));
}

int unlock_sb(Sem_binar* sb){
    atomic_flag_clear(&sb->flg);
}

void sem_binar_destroy(Sem_binar* sb){
    atomic_flag_clear(&sb->flg);
}



//                                     RWLOCK
//  ====================================================================================

void rwl_init(Rwlock* rwl)
{
    rwl->counter = 0;
    sem_binar_init(&rwl->counter_lock);
    spn_init(&rwl->write_lock);
}

void lock_for_reading(Rwlock* rwl)
{   
    lock_sb(&rwl->counter_lock);
    rwl->counter++;
    if(rwl->counter == 1)
        spn_lock(&rwl->write_lock);
    unlock_sb(&rwl->counter_lock);
}

void unlock_for_reading(Rwlock* rwl)
{   
    lock_sb(&rwl->counter_lock);
    rwl->counter--;
    if(rwl->counter == 0)
        spn_unlock(&rwl->write_lock);
    unlock_sb(&rwl->counter_lock);
}

void lock_for_writing(Rwlock* rwl)
{
    spn_lock(&rwl->write_lock);
}

void unlock_for_writing(Rwlock* rwl)
{   
    spn_unlock(&rwl->write_lock);
}

void rwl_destroy(Rwlock* rwl)
{
    sem_binar_destroy(&rwl->counter_lock);
    spn_destroy(&rwl->write_lock);
}


//                          MUTEX - LAMPORT ALGORITHM, N Threads
//  ===================================================================================

void nmut_init(NMut* mut, int NrThreads){
    mut->thr_start = syscall(__NR_gettid);
    mut->ordin = malloc(NrThreads*sizeof(int));
    mut->acum_alege = malloc(NrThreads*sizeof(int));
    mut->nrThreads = NrThreads;
}

void _nmut_init(NMut* mut){
    mut->thr_start = syscall(__NR_gettid);
    mut->ordin = malloc(100*sizeof(int));
    mut->acum_alege = malloc(100*sizeof(int));
    mut->nrThreads = 100;
}

void nmut_lock(NMut* mut){
    int actual = syscall(__NR_gettid) - mut->thr_start - 1; //ca sa numeroteze de la 0 in vector
    mut->acum_alege[actual] = 1;     //acum isi cauta locul la coada

    int mx = 0;
    for (int i = 0;i < mut->nrThreads; i++) {
        int nr = mut->ordin[i];   //il scoatem pt atomicitatea comparatiei ce urmeaza
        if(nr > mx)
            mx = nr;
    }

    mut->ordin[actual] = mx+1;     // practic il punem ultimul in coada
    mut->acum_alege[actual] = 0;

    for (int j = 0;j < mut->nrThreads; j++) {
        while (mut->acum_alege[j]) {}    //asteapta sa-si gaseasca locul la coada
        while (mut->ordin[j] != 0 && (mut->ordin[j] < mut->ordin[actual] || (mut->ordin[j] == mut->ordin[actual] && j < actual))){}
    }
}

void nmut_unlock(NMut* mut){
    mut->ordin[syscall(__NR_gettid)- mut->thr_start - 1] = 0;
}
  
void nmut_destroy(NMut* mut){
    free(mut->ordin);
    free(mut->acum_alege);
}
 


//                                    SEMAFOR
//  ====================================================================================

 
void _s_init(Semafor* s, int val){
    s->size = val;
    s->count = 0;
    _nmut_init(&s->mtx);
}

void s_init(Semafor* s, int val, int nrthr){
    s->size = val;
    s->count = 0;
    nmut_init(&s->mtx, nrthr);
}

void s_wait(Semafor* s){
    while(1){
        while (atomic_load(&s->count) >= s->size) {}
        nmut_lock(&s->mtx);
        if (s->count >= s->size) {
            nmut_unlock(&s->mtx);
            continue;
        }
        s->count++;
        nmut_unlock(&s->mtx);
        break;
    }
}

void s_post(Semafor* s){
    nmut_lock(&s->mtx);
    if (s->count > 0)
        s->count--;
    nmut_unlock(&s->mtx);
}

int s_getvalue(int* dest, Semafor* s){
    nmut_lock(&s->mtx);
    *dest = s->size - s->count;
    nmut_unlock(&s->mtx);
}

void s_destroy(Semafor* s){
    nmut_destroy(&s->mtx);
}


//                                    BARIERA
//  ====================================================================================



void _bar_init(Bariera* bar, int n){     // cu 100 threaduri
    bar->ajunse = 0;
    bar->go = 0;
    _s_init(&bar->semaphore, 1);
    _nmut_init(&bar->mutex);
    bar->nrThr = n;
}

void bar_init(Bariera* bar,int n, int nrThre){       // cu nr de threaduri maxim specificat
    bar->ajunse = 0;
    bar->go = 0;
    s_init(&bar->semaphore, 1, nrThre);
    nmut_init(&bar->mutex, nrThre);
    bar->nrThr = n;
}

void bar_point(Bariera* bar){   //a ajuns la bariera
    nmut_lock(&bar->mutex);
    if (bar->ajunse == 0)    //coboara bariera cand se ajunge la 0 la coada
        s_wait(&bar->semaphore);
    bar->ajunse++;
    nmut_unlock(&bar->mutex);
    if(bar->ajunse == bar->nrThr){   //ridica bariera cand se ajunge la nr bun
        bar->ajunse = 0;
        s_post(&bar->semaphore);
    }
    else{
        int val;
        s_getvalue(&val, &bar->semaphore);
        while(!val)
            s_getvalue(&val, &bar->semaphore);  //se invarte pana se elibereaza bariera
    }

}

void bar_destroy(Bariera* bar){
    nmut_destroy(&bar->mutex);
    s_destroy(&bar->semaphore);
}