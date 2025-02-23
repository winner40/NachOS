// addrspace.h
//      Data structures to keep track of executing user programs
//      (address spaces).
//
//      For now, we don't keep any information about address spaces.
//      The user level CPU state is saved and restored in the thread
//      executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "translate.h"

#define UserStackSize 1048 // increase this as necessary!

// MULTI-THREADING PURPOSE
#include "bitmap.h"

class Lock;
class Condition;

class AddrSpace {
  public:
    AddrSpace(OpenFile *executable); // Create an address space,
    // initializing it with the program
    // stored in the file "executable"
    ~AddrSpace(); // De-allocate an address space

    void InitRegisters(); // Initialize user-level CPU registers,
    // before jumping to user code

    void SaveState();    // Save/restore address space-specific
    void RestoreState(); // info on a context switch

    int GetNumThreads();
    unsigned int GetNextThreadID();
		bool IsCreated() ;
    int processID;
    unsigned int max_threads;

    /* Resource management */
    /* Methods for Thread Table management */
    void UnlockThreadTable();
    int LockThreadTable();
    void AddThread(unsigned int ID);
    void RemoveThread(unsigned int ID);
    bool HasThread(unsigned int ID);

    /* Methods for Thread Stack management */
		void LockThreadStack() ;
		void UnlockThreadStack() ;
		int GetStackPointer() ;
		void RemoveStackPointer(int i) ;

    /* Synch management */
    /* Methods for waiting a thread */
		void InitJoinConditions() ;
		void DeleteJoinConditions() ;
		void WaitJoinCondition(unsigned int ID) ;
		void BroadcastJoinCondition(unsigned int ID) ;

    /* Methods for exiting a thread */
		void WaitExitCondition() ;
		void BroadcastExitCondition() ;

    /* Methods for frame management */
    void FreeFrames() ;

    /* Init function */
    void InitSpaceSetup();

  private:

    TranslationEntry *pageTable; 
    unsigned int numPages; 
    bool isSpaceCreated;                    // represents whether the address space has been successfully created
    unsigned int nb_threads;                // total number of threads accomodable in the address space
    unsigned int thread_counter;            // counter for assigning unique thread ID throughout the address space life
    BitMap* threadStackBitmap;              // manages the allocation of stack regions for the threads of the same address space
    Lock* threadTableLock;                  // lock to protect access to the thread Table
    Lock* threadStackBitmapLock;             // lock to protect acces to the thread stack table
    Condition *threadExitCond;              // condition variable used to manage synchronization when threads exit
    unsigned int* threadTable;              // arrray that stores the IDs of the thread currently active in the address space
    Condition** threadSynchTable;           // array of condition variable for thread synchronization, mainly used for coordination and wait

};

void copyStringFromMachine(int from, char *to, unsigned size);
int copyStringToMachine(char *from, int to, unsigned int size);

#endif // ADDRSPACE_H
