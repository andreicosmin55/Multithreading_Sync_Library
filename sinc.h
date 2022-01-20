#ifndef SINC_H_
#define SINC_H_
#include <stdatomic.h>
#include <pthread.h>  //pt debug
//                                   SPINLOCK cu ownership
//  ===================================================================================
typedef struct{
    volatile _Atomic int owner;
} Spn;

void spn_init(Spn* Mut);
void spn_lock(Spn* Mut);
int spn_unlock(Spn* Mut);
int spn_destroy(Spn* Mut);



//                                  SEMAFOR BINAR
//  ====================================================================================
typedef struct{
    volatile atomic_flag flg;
} Sem_binar;

void sem_binar_init(Sem_binar* sb);
void lock_sb(Sem_binar* sb);
int unlock_sb(Sem_binar* sb);
void sem_binar_destroy(Sem_binar* sb);

//                                     RWLOCK
//  ====================================================================================

typedef struct{
    Spn write_lock;
    Sem_binar counter_lock;
    int counter;
} Rwlock;

void rwl_init(Rwlock* rwl);
void lock_for_reading(Rwlock* rwl);
void unlock_for_reading(Rwlock* rwl);
void lock_for_writing(Rwlock* rwl);
void unlock_for_writing(Rwlock* rwl);
void rwl_destroy(Rwlock* rwl);


//                                   MUTEX - LAMPORT ALGORITHM, N processes
//  ===================================================================================

typedef struct{
    int thr_start;
    volatile int* ordin;
    volatile int* acum_alege;
    int nrThreads;
} NMut;

void _nmut_init(NMut* mut);
void nmut_init(NMut* mut, int NrThreads);
void nmut_lock(NMut* mut);
void nmut_unlock(NMut* mut);
void nmut_destroy(NMut* mut);



//                                    SEMAFOR
//  ====================================================================================

typedef struct{
    int size;
    _Atomic int count;
    NMut mtx;
} Semafor;

void _s_init(Semafor* s, int val);
void s_init(Semafor* s, int val, int nrthr);
void s_wait(Semafor* s);
void s_post(Semafor* s);
int s_getvalue(int* dest, Semafor* s);
void s_destroy(Semafor* s);


//                                    BARIERA
//  ====================================================================================

typedef struct{
    int nrThr;
    NMut mutex;
    Semafor semaphore;
    int ajunse, go;
} Bariera;

void _bar_init(Bariera* bar, int n);
void bar_init(Bariera* bar,int n, int nrThre);
void bar_point(Bariera* bar);
void bar_destroy(Bariera* bar);

#endif