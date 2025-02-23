#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "userthread.h"
#include "threadparams.h"
#include <stdio.h>

//--------------------------------------------------------------------------
// StartUserThread
//			Sets the thread's execution context and start it
//
//			This function initializes registers specific to the thread's address
//			space, sets up the user program counter and its stack pointer, then
//			run the newly created thread. Does not return to the calling context,
//			once machine->Run is called, the control switches entirely
//			
//			Arguments:
//				f: a pointer to the an objects containaing the information about 
//					the thread like the function to execute, its arguments and 
//					the return function
//---------------------------------------------------------------------------

static void StartUserThread(int f)
{
	ThreadParams *params = (ThreadParams*) f ;

	currentThread->space->InitRegisters() ;
	currentThread->space->RestoreState() ;

	machine->WriteRegister(4, params->functionArgs) ;
	machine->WriteRegister(PCReg, params->threadFunction) ;
	machine->WriteRegister(NextPCReg, params->threadFunction + 4) ;
	machine->WriteRegister(RetAddrReg, params->returnFunction) ;
    machine->WriteRegister(StackReg, currentThread->GetStackPointer()) ;

	machine->Run() ;
	ASSERT(FALSE) ;	 	
}

//--------------------------------------------------------------------------
// do_UserThreadCreate
//			Creates a new user thread to execute the specified function.
//
//			Sets the address space of the new thread to the address space of the
//			parent thread. Increments the number of thread using the address space
//			checks if we don't exceed the number of threads accommodable. sets the
//			identifier of the new thread and adds it to the table of tracked thread
//			Allocates a stack to the thread and create a corresponding kernel thread.
//			Uses a synchronization to access threadTable and to ensure the exclusivity
//			of stack allocation.
//
//			Arguments:
//				fun: the address of the function to be executed by the thread
//				arg: the arguments to pass the thread function
//				returnFun: the return function after the thread finishes
//
//			Returns:
//				An integer representing the thread ID of the newly created thread,
//				or -1 if the thread creation fails
//---------------------------------------------------------------------------

int do_UserThreadCreate(int fun, int arg, int returnFun)
{
	AddrSpace *currentSpace = currentThread->space ;
	currentSpace->LockThreadTable() ;

	int totalThreads = currentSpace->GetNumThreads() + 1 ;
	if ((unsigned)totalThreads > currentThread->space->max_threads)
	{
		fprintf(stderr, "Error in do_UserThreadCreate: Maximum number of threads reached\n");
		currentSpace->UnlockThreadTable() ;
		return -1 ;
	}

	int tid = currentSpace->GetNextThreadID() ;
	currentSpace->AddThread(tid) ;
	currentSpace->UnlockThreadTable() ;

	currentSpace->LockThreadStack() ;
    int sp = currentSpace->GetStackPointer() ;
	if(sp == -1){
		fprintf(stderr, "Error in do_UserThreadCreate: Not enough space to run thread!! \n");
		return -1;
	}
	currentSpace->UnlockThreadStack() ;

	ThreadParams *params = new ThreadParams(fun, arg, returnFun, true) ;
	char *name = (char *) malloc(sizeof(char) * 18) ; // max string length 18
	sprintf(name, "User thread n°%d", tid) ;

	Thread *thread = new Thread(name, tid, sp) ;
	thread->space = currentSpace ;
	thread->Fork(StartUserThread, (int) params) ;

	return tid ; 
}

//--------------------------------------------------------------------------
// do_UserThreadExit
//			Exits the current user thread and performs any necessary cleanup.
//
//			Retrieve the actual space and remove the current thread from the 
//			table of active threads. Remove the tracking of the stack pointer
//			and frees allocated stack.
//			uses synchronization to operate on threadTable and to threadStack
//			table.
//
//			This function does not take any arguments or return any value
//---------------------------------------------------------------------------

void do_UserThreadExit()
{
	AddrSpace *currentSpace = currentThread->space ;

	if (currentSpace->LockThreadTable() == 0){
		currentSpace->RemoveThread(currentThread->GetThreadID());
		currentSpace->UnlockThreadTable() ;

		currentSpace->LockThreadStack() ;
		currentSpace->RemoveStackPointer(currentThread->GetStackPointer()) ;
		currentSpace->UnlockThreadStack() ;
	}

	currentThread->Finish();
	delete currentThread->space ;
}

//--------------------------------------------------------------------------
// do_UserThreadJoin
//			Waits for the thread with the specified identifier t to finish 
//			execution. 
//
//			This function checks if the given thread is not the current thread
//			and is still still active, then synchronize calling thread with 
//			targetted ones. Uses synchronisation to control access to threadTable
//
//			Arguments:
//				t: The identifier of the thread to wait
//			
//			Returns:
//				0 if the operation is successful or -1 if it fails
//---------------------------------------------------------------------------

int do_UserThreadJoin(int t) 
{
	AddrSpace *currentSpace = currentThread->space ;
	
	if(currentThread->GetThreadID() == t || !currentSpace->HasThread(t)){ return -1; }

	currentSpace->LockThreadTable() ;
	while (currentSpace->HasThread(t))
	{
		currentSpace->WaitJoinCondition(t) ;
	}
	currentSpace->UnlockThreadTable() ;
	
	return 0 ;
}

//--------------------------------------------------------------------------
// do_UserThreadGetID
//			Returns the current thread identifier
//			
//			Returns:
//				an integer representing the id of the current thread
//---------------------------------------------------------------------------

int do_UserThreadGetID()
{
	return currentThread->GetThreadID() ;
}
