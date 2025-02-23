// system.h
//      All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef SYSTEM_H
#define SYSTEM_H

#include "copyright.h"
#include "interrupt.h"
#include "scheduler.h"
#include "stats.h"
#include "thread.h"
#include "timer.h"
#include "utility.h"
#include "synchconsole.h"
#include "frameprovider.h"

// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); // Initialization,
                                               // called before anything else
extern void Cleanup();                         // Cleanup, called when
                                               // Nachos is done.

extern Thread *currentThread;       // the thread holding the CPU
extern Thread *threadToBeDestroyed; // the thread that just finished
extern Scheduler *scheduler;        // the ready list
extern Interrupt *interrupt;        // interrupt status
extern Statistics *stats;           // performance metrics
extern Timer *timer;                // the hardware alarm clock

#ifdef USER_PROGRAM
#include "machine.h"
extern Machine *machine;                                // user program memory and registers
extern SynchConsole * synchConsole;                     // the interface for synchronised console I/O

// VIRTUAL MEMORY AND MULTIPLE PROCESS
#define TEMP_MAXPROC_NUMBER 64                          // Maximum number of processes supported
#define MAX_FILENAME 100                                // Maximum length of file names

extern FrameProvider *frameProvider;                    // Manages allocation and tracking of physical memory
extern Lock* processLocks[TEMP_MAXPROC_NUMBER];         // Locks to synchronize access to process resources
extern Condition* processConds[TEMP_MAXPROC_NUMBER];    // Conditions for inter-process communication
extern int processTable[TEMP_MAXPROC_NUMBER];           // Tracks active processes

extern unsigned int numProcess;                         // Tracks the number of active processes
extern Lock *processLock;                               // Lock to protect the process counter 'numProcess'

extern void AcquireProcessLock();                       // Function to acquire the process lock
extern void ReleaseProcessLock();                       // Function to release the process lock
#endif

#ifdef FILESYS_NEEDED // FILESYS or FILESYS_STUB
#include "filesys.h"
extern FileSystem *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk *synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice *postOffice;
#endif

//adding macros for synchronous calls
#define MAX_STRING_SIZE 255 // max number of characters for string buffer
#define MAX_INT_SIZE 15 // max number of digits for an integer buffer

#endif // SYSTEM_H
