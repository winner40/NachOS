#include "copyright.h"
#include "syscall.h"
#include "userprocess.h"
#include "threadparams.h"
#include <stdio.h>

//-------------------------------------------------------------------------------
// StartUserThread
//			Sets the thread's execution context and start it
//
//			This function initializes registers specific to the thread's address
//			space then run the newly created thread. Does not return to the 
//			calling context, once machine->Run is called, the control switches 
//			entirely
//			
//			Arguments:
//				f: a pointer to the an objects containaing the information about 
//					the thread like the function to execute, its arguments and 
//					the return function
//-------------------------------------------------------------------------------

static void StartUserProcess(int f) 
{

	currentThread->space->InitRegisters() ;
	currentThread->space->RestoreState() ;

	machine->Run() ;

	ASSERT(FALSE) ;
}

//--------------------------------------------------------------------------
// do_UserProcessCreate
//			Creates a new user process to execute a program.
//
//			Creates and assign a new address space for the new process, increments
//			the number of process of the system, creates a new thread to execute
//			the content of the program and runs it.
//
//			Arguments:
//				filename: the name of the file containing the program to execute
//				arg: the arguments to pass the program being executed
//				returnFun: the return function after the process ends
//
//			Returns:
//				An integer representing the identifier of the new process,
//				or -1 if the process creation fails
//---------------------------------------------------------------------------

int do_UserProcessCreate(char *filename, int arg, int returnFun)
{
    
	OpenFile *file = fileSystem->Open(filename) ;
	if (file == NULL) 
	{
		fprintf(stderr, "Error in do_UserThreadCreate: Couldn't not open file %s \n", filename);
		return -1 ;
	}

	AddrSpace *addrSpace = new AddrSpace(file) ;
	if (! addrSpace->IsCreated())
	{
		fprintf(stderr, "Error in do_userThreadCreate: Could not create address space for file! \n");
		delete addrSpace ;
		return -1 ;
	}

	AcquireProcessLock() ;
	numProcess++;
	ReleaseProcessLock() ;

	
	ThreadParams *params = new ThreadParams((int)filename, 0, 0, false);
	Thread *thread = new Thread(filename, 1, 0) ;
	thread->space = addrSpace ; 
	thread->Fork(StartUserProcess, (int) params) ;

	return addrSpace->processID ;
}

//--------------------------------------------------------------------------
// do_UserProcessExit
//			Exit the current user process, suspend the processs if it has 
//			children threads that are still active. Decreases the number of 
//			process of the system and halt all activites of the current process
//			Mark the process as not active and signal all other process waiting
//			on it then delete its associated address space
//
//			This function takes no argument and returns no value.
//---------------------------------------------------------------------------

void do_UserProcessExit() 
{
	AddrSpace *currentSpace = currentThread->space ;
	int pID = currentSpace->processID;

	waitForChildrenToExit(currentSpace);

	AcquireProcessLock() ;
	int numProc = --numProcess;
	ReleaseProcessLock() ;

	if (numProc == 0)
	{
		fprintf(stderr, "Trace in do_UserThreadExit: Last process finished! \n");
		DEBUG('a', "The process ended up correctly.\n");
		interrupt->Halt() ;
		return;
	}

	cleanUpOnExit(pID, currentSpace);
	currentThread->Finish();
}

//--------------------------------------------------------------------------
// do_WaitProcess
//			Wait for a specific process to finish execution before continuing
//			execution. This function will suspend the calling processs using
//			condition variable
//
//			Arguments:
//				processID: the identifier of the process to wait for
//---------------------------------------------------------------------------

void do_WaitProcess(int processID)
{
	if (processID == currentThread->space->processID) {
		fprintf(stderr, "Error in do_WaitProcess: cannot wait himself!");
		return;
	}

	processLocks[processID]->Acquire();
	while(processTable[processID]){
		processConds[processID]->Wait(processLocks[processID]);
	}
	processLocks[processID]->Release();
}

//--------------------------------------------------------------------------
// do_UserProcessHalt
//			Halt the current user process and terminates its execution.
//
//			This function takes no argument and returns no value.
//---------------------------------------------------------------------------
void do_UserProcessHalt()
{
	DEBUG('a', "Shutdown, initiated by user program.\n");
	interrupt->Halt() ;
}

//--------------------------------------------------------------------------
// Miscellanous functions
//			For better structuring.
//
//---------------------------------------------------------------------------

void waitForChildrenToExit(AddrSpace *space){

	space->LockThreadTable();
	while (space->GetNumThreads() > 1)
	{
		space->WaitExitCondition() ;
	}
	space->UnlockThreadTable() ;
}

void cleanUpOnExit(int ID, AddrSpace *space){

	processLocks[ID]->Acquire();
	processTable[ID] = 0;
	processConds[ID]->Broadcast(processLocks[ID]);
	processLocks[ID]->Release();
	delete space ;

}
