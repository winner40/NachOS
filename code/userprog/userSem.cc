#include "userSem.h"

Semaphore *SemInit(int val){
    return new Semaphore("userSem", val);
}

void SemP(Semaphore *sem){
    sem->P();
}

void SemV(Semaphore *sem){
    sem->V();
}