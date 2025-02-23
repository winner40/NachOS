// synch.cc
//      Routines for synchronizing threads.  Three kinds of
//      synchronization routines are defined here: semaphores, locks
//      and condition variables (the implementation of the last two
//      are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "synch.h"
#include "copyright.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
//      Initialize a semaphore, so that it can be used for synchronization.
//
//      "debugName" is an arbitrary name, useful for debugging.
//      "initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(const char *debugName, int initialValue) {
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
//      De-allocate semaphore, when no longer needed.  Assume no one
//      is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore() { delete queue; }

//----------------------------------------------------------------------
// Semaphore::P
//      Wait until semaphore value > 0, then decrement.  Checking the
//      value and decrementing must be done atomically, so we
//      need to disable interrupts before checking the value.
//
//      Note that Thread::Sleep assumes that interrupts are disabled
//      when it is called.
//----------------------------------------------------------------------

void Semaphore::P() {
    IntStatus oldLevel = interrupt->SetLevel(IntOff); // disable interrupts

    while (value == 0) {                      // semaphore not available
        queue->Append((void *)currentThread); // so go to sleep
        currentThread->Sleep();
    }
    value--; // semaphore available,
    // consume its value

    (void)interrupt->SetLevel(oldLevel); // re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
//      Increment semaphore value, waking up a waiter if necessary.
//      As with P(), this operation must be atomic, so we need to disable
//      interrupts.  Scheduler::ReadyToRun() assumes that threads
//      are disabled when it is called.
//----------------------------------------------------------------------

void Semaphore::V() {
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL) // make thread ready, consuming the V immediately
        scheduler->ReadyToRun(thread);
    value++;
    (void)interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments
// Note -- without a correct implementation of Condition::Wait(),
// the test case in the network assignment won't work!

//---------------------------------------------------------------------------------------------
// Lock::Lock
//			Constructor that initializes a lock to use for synchronization
//
//			Initilazes two semaphores a general lock and an inner lock to ensure exclusive 
//          access to the operations and resources. Sets the ownerID to -1 to specify that
//          no thread holds it yet
//			
//			Arguments:
//				debugName: arbitrary name given to the lock
//----------------------------------------------------------------------------------------------

Lock::Lock(const char *debugName) {
    name = debugName;
    ownerID = -1;
    lock = new Semaphore("Lock semaphore", 1);
    mutex = new Semaphore("Lock inner semaphore", 1);
}

//---------------------------------------------------------------------------------------------
// Lock::~Lock
//			Destructor that deletes resources allocated to a lock
//
//			This function does not take arguments and returns no value
//----------------------------------------------------------------------------------------------

Lock::~Lock() {
    delete lock;
    delete mutex;
}

//---------------------------------------------------------------------------------------------
// Lock::Acquire
//			Atomically wait for the lock to be free and then set it to busy
//          Sets the holder of the lock to the actual thread ID, else we release
//          the lock.
//
//			This function does not take arguments and returns no value
//----------------------------------------------------------------------------------------------

void Lock::Acquire() {

    lock->P();

    mutex->P();
    if (ownerID == -1){ ownerID = currentThread->GetThreadID(); }
	else { lock->V() ;}
	mutex->V() ;
}

//---------------------------------------------------------------------------------------------
// Lock::Acquire
//			Atomically set the lock to be free. Wakes a waiting thread if any.
//          Sets the holder of the lock to -1 to reflect that nobody holds it
//          Only the thread that holds it can release it
//
//			This function does not take arguments and returns no value
//----------------------------------------------------------------------------------------------

void Lock::Release() {
    mutex->P();

    if(ownerID == currentThread->GetThreadID()){
        ownerID = -1;
        lock->V();
    }

    mutex->V();
}

//---------------------------------------------------------------------------------------------
// Condition::Condition
//			Constructor that initializes a condition variable so that it can be used
//          for synchronization. Initially no one is waiting on the condition.
//
//			Arguments:
//              debugname:  an arbitrary name for the condition variable
//----------------------------------------------------------------------------------------------


Condition::Condition(const char *debugName) {
    name = debugName;
    waitingThreads = 0;
    sleepLock = new Semaphore("Sleeping semaphore", 0);
    mutex = new Semaphore("Inner lock semaphore", 1);
}

//---------------------------------------------------------------------------------------------
// Condition::~Condition
//			Destructor that deallocates the resources given to a condition variable
//
//          This function takes no argument, and returns no value
//----------------------------------------------------------------------------------------------

Condition::~Condition() {
    delete sleepLock;
    delete mutex;
}

void Condition::Wait(Lock *conditionLock) { 
    mutex->P();

    waitingThreads++;
    conditionLock->Release();
    mutex->V();
    sleepLock->P();
    conditionLock->Acquire();
 }


//---------------------------------------------------------------------------------------------
// Condition::Signal
//			Wakes up one single thread that is waiting on the conditional variable
//
//			Acquires and releases the internal lock to ensure exclusive access to the condition
//          variable during the operation. If there is at least one thread waiting on the 
//          CV releases the associated semaphore and decrements the number of sleeping threads
//			
//			Arguments:
//				conditionLock: a lock associated with the condition variable that
//                  will be held by the caller
//----------------------------------------------------------------------------------------------

void Condition::Signal(Lock *conditionLock) {
    mutex->P();

    if(waitingThreads > 0){
        conditionLock->Release(); // IF ERROR COME BACK HERE
        sleepLock->V();
        waitingThreads--;
        conditionLock->Acquire();
    }

    mutex->V();
}

//---------------------------------------------------------------------------------------------
// Condition::Broadcast
//			Wakes up all threads that are waiting on the conditional variable
//
//			Acquires and releases the internal lock to ensure exclusive access to the condition
//          variable during the operation. Iterates through all the threads waiting
//          on the CV and signals each waiting thread by releasing the associated 
//          semaphore and decrements the number of sleeping threads
//			
//			Arguments:
//				conditionLock: a lock associated with the condition variable that
//                  will be held by the caller
//----------------------------------------------------------------------------------------------

void Condition::Broadcast(Lock *conditionLock) {
    mutex->P();
    while(waitingThreads > 0){
        conditionLock->Release();
        sleepLock->V();
        waitingThreads--;
        conditionLock->Acquire();
    }
    mutex->V();
}
