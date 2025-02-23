#include "synch.h"

Semaphore *SemInit(int val);

void SemP(Semaphore *sem);

void SemV(Semaphore *sem);