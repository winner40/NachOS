// addrspace.cc
//      Routines to manage address spaces (executing user programs).
//
//      In order to run a user program, you must:
//
//      1. link with the -N -T 0 option
//      2. run coff2noff to convert the object file to Nachos format
//              (Nachos object code format is essentially just a simpler
//              version of the UNIX executable object code format)
//      3. load the NOFF file into the Nachos file system
//              (if you haven't implemented the file system yet, you
//              don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "addrspace.h"
#include "copyright.h"
#include "noff.h"
#include "system.h"

#include "synch.h"

#include <strings.h> /* for bzero */

#define UserThreadStackSize (PageSize * 2)
#define UserThreadMax UserStackSize / UserThreadStackSize

static void ReadAtVirtual(OpenFile *executable, int virtualaddr, 
		int numBytes, int position, 
		TranslationEntry *pageTable, unsigned numPages) ; 

//----------------------------------------------------------------------
// SwapHeader
//      Do little endian to big endian conversion on the bytes in the
//      object file header, in case the file was generated on a little
//      endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void SwapHeader(NoffHeader *noffH) {
    noffH->noffMagic = WordToHost(noffH->noffMagic);
    noffH->code.size = WordToHost(noffH->code.size);
    noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
    noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
    noffH->initData.size = WordToHost(noffH->initData.size);
    noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
    noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
    noffH->uninitData.size = WordToHost(noffH->uninitData.size);
    noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
    noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
//      Create an address space to run a user program.
//      Load the program from a file "executable", and set everything
//      up so that we can start executing user instructions.
//
//      Assumes that the object code file is in NOFF format.
//
//      First, set up the translation from program memory to physical
//      memory.  For now, this is really simple (1:1), since we are
//      only uniprogramming, and we have a single unsegmented page table
//
//      "executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable) {
    NoffHeader noffH;
    unsigned int i, size;


    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) &&
        (WordToHost(noffH.noffMagic) == NOFFMAGIC))
        SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);



    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size +
           UserStackSize; // we need to increase the size
    // to leave room for the stack

    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;

    ASSERT(numPages <= NumPhysPages); // check we're not trying
    // to run anything too big --
    // at least until we have
    // virtual memory


    DEBUG('a', "Initializing address space, num pages %d, size %d\n", numPages,
          size);
    // first, set up the translation
    pageTable = new TranslationEntry[numPages];


    if (frameProvider->NumAvailFrame() < numPages)
	{
        fprintf(stderr, "Error in addrSpace:  Not Enough frames to allocate.\n");
		isSpaceCreated = false ;
		return ; 
	}


    for (i = 0; i < numPages; i++) {

        if (!frameProvider->IsFrameAvail()) {
			fprintf(stderr, "Error in AddrrSpace: No more frame available. \n");
			return ;
		}
		
		pageTable[i].physicalPage = frameProvider->GetEmptyFrame() ;
        pageTable[i].virtualPage = i; // for now, virtual page # = phys page #
        // pageTable[i].physicalPage = i + 1;
        pageTable[i].valid = TRUE;
        pageTable[i].use = FALSE;
        pageTable[i].dirty = FALSE;
        pageTable[i].readOnly = FALSE; // if the code segment was entirely on
                                       // a separate page, we could set its
                                       // pages to be read-only
    }

    // zero out the entire address space, to zero the unitialized data segment
    // and the stack segment
    // bzero(machine->mainMemory, size);

    // then, copy in the code and data segments into memory
    if (noffH.code.size > 0) {
		DEBUG ('a', "Initializing code segment, at 0x%x, size %d\n",
				noffH.code.virtualAddr, noffH.code.size);
		ReadAtVirtual(executable, noffH.code.virtualAddr, noffH.code.size, 
				noffH.code.inFileAddr, pageTable, numPages) ;
	}
	if (noffH.initData.size > 0) {
		DEBUG ('a', "Initializing data segment, at 0x%x, size %d\n",
				noffH.initData.virtualAddr, noffH.initData.size);
		ReadAtVirtual(executable, noffH.initData.virtualAddr, noffH.initData.size, 
				noffH.initData.inFileAddr, pageTable, numPages);
	}

    InitSpaceSetup();
	isSpaceCreated = true ;
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
//      Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace() {
    // LB: Missing [] for delete
	FreeFrames();
    // delete pageTable;
    delete[] pageTable;

    delete threadStackBitmap;
    delete threadTableLock;
    delete threadStackBitmapLock;
    delete threadExitCond;

    DeleteJoinConditions();
    // delete[] threadWait;
    // End of modification
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
//      Set the initial values for the user-level register set.
//
//      We write these directly into the "machine" registers, so
//      that we can immediately jump to user code.  Note that these
//      will be saved/restored into the currentThread->userRegisters
//      when this thread is context switched out.
//----------------------------------------------------------------------

void AddrSpace::InitRegisters() {
    int i;

    for (i = 0; i < NumTotalRegs; i++)
        machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

    // Set the stack register to the end of the address space, where we
    // allocated the stack; but subtract off a bit, to make sure we don't
    // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
//      On a context switch, save any machine state, specific
//      to this address space, that needs saving.
//
//      For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() {
    pageTable = machine->pageTable;
    numPages = machine->pageTableSize;
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
//      On a context switch, restore the machine state so that
//      this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() {
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}

// INIT STRUCTURE PURPOSE

//----------------------------------------------------------------------
//  AddrSpace::InitSpaceSetup
//		Initializes the resources and synch primitives used by the address
//		space during its lifetime
//
//		This function sets up the thread table and thread condition
//		variables, sets up the stack table and bitmap. Sets up the related
//		synch mechanisms to control access to these resources. Initializes
//		the counters of threads and assign the next process identifier
//
//		This function takes no argument and return no value
//----------------------------------------------------------------------
void AddrSpace::InitSpaceSetup(){
	
	threadTable = new unsigned int[UserThreadMax];      
    threadSynchTable = new Condition*[UserThreadMax]; 
    threadStackBitmap =  new BitMap(UserThreadMax);

	threadTableLock = new Lock("AddrSpace threads id lock") ;
	threadStackBitmapLock = new Lock("AddrSpace threads stack lock") ;
	threadExitCond = new Condition("AddrSpace threads exit cond") ;
	InitJoinConditions() ;

	nb_threads = 1 ;
    max_threads = UserThreadMax - 1;
	threadTable[0] = 1 ;
	thread_counter = 1 ;

	for(int i = 1; i < TEMP_MAXPROC_NUMBER; i++){
        processLocks[i]->Acquire();

        if(processTable[i] != 1){
            processID = i;
            processTable[i] = 1;
            processLocks[i]->Release();
            break;
        }

        processLocks[i]->Release();
    }
}

void AddrSpace::InitJoinConditions()
{
	for (unsigned int i = 0 ; i < UserThreadMax ; i ++)
	{
		char *name = (char *) malloc(sizeof(char) * 50) ;
		sprintf(name, "Addrspace thread join condition n°%d", i) ;

		threadSynchTable[i] = new Condition(name) ;
	}
}

void AddrSpace::DeleteJoinConditions()
{
	for (unsigned int i = 0 ; i < UserThreadMax ; i ++)
	{
		delete threadSynchTable[i] ; 
	}
}

// MULTI-THREADING PURPOSE
//----------------------------------------------------------------------
// Functions to manage the user threads. 
//		Every access to a "get" or "set" function must be protected with
//		the following locks.
//----------------------------------------------------------------------

// ----------------------------------------------------------------
// 	AddrSpace::GetNumThreads
//		Returns the number of active threads using the address space
// -----------------------------------------------------------------
int AddrSpace::GetNumThreads() { return nb_threads ; }

// ----------------------------------------------------------------
// 	AddrSpace::AddThread
//		Append a thread identifier to the table of active threads
//		Do nothing if the identifier is already in the table
// -----------------------------------------------------------------
void AddrSpace::AddThread(unsigned int ID)
{
	if (HasThread(ID)) return ;
	threadTable[nb_threads] = ID ;
	nb_threads ++ ;
}

// ----------------------------------------------------------------
// 	AddrSpace::RemoveThread
//		Remove a thread identifier from the table of active threads
//		Do nothing if the identifier is not in the table in the table
//
//		Looks for the matching entry of the target thread and updates
//		the number of active threads, signals the threads waiting for
//		identified thread, signals globally that a thread was terminated
// -----------------------------------------------------------------
void AddrSpace::RemoveThread(unsigned int ID)
{
	for (unsigned int i = 0 ; i < nb_threads ; i ++)
	{
		if (threadTable[i] == ID)
		{
			for (unsigned int j = i ; j < nb_threads - 1 ; j ++)
			{
				threadTable[j] = threadTable[j + 1] ;
			}

			nb_threads -- ;
			BroadcastJoinCondition(ID) ;
			BroadcastExitCondition() ;
			return ;
		}
	}
}

// ----------------------------------------------------------------
// 	AddrSpace::HasThread
//		Returns true if the given thread identifier is in the table
//		of active thread, else returns false
// -----------------------------------------------------------------
bool AddrSpace::HasThread(unsigned int ID)
{
	for (unsigned int i = 0 ; i < nb_threads ; i ++)
	{
		if (threadTable[i] == ID) return true ;
	}
	return false ; 
}

// ----------------------------------------------------------------
// 	AddrSpace::GetNextThreadID
//		Generates a unique thread identifier throughout the lifetime
//		of the address space. uses a thread_counter to track the 
//		number of ever created threads associated 
// -----------------------------------------------------------------
unsigned int AddrSpace::GetNextThreadID()
{
	int n = thread_counter;
    thread_counter++;
    return n ;  
}

// ----------------------------------------------------------------
// 	AddrSpace::GetStackPointer
//		Allocates and returns a stack pointer for a new thread in
//		the address space, using a bitmap to manage available stack
//
//		when a free slot is found in the bitmap, marks it as used 
//		and compute the corresponding stack pointer
// -----------------------------------------------------------------
int AddrSpace::GetStackPointer()
{

    int next_free;
    if((next_free = threadStackBitmap->Find()) != -1){
        return numPages * PageSize - ((next_free + 1) * UserThreadStackSize) ;  
    }
	return -1 ;
}

// ----------------------------------------------------------------
// 	AddrSpace::GetStackPointer
//		Frees a thread stack slot in the bitmap to mark it as reusable
//
//		computes the corresponding stack index in the bitmap using 
//		a given stack pointer of a thread we want to deallocate
// -----------------------------------------------------------------
void AddrSpace::RemoveStackPointer(int sp)
{
	int i = ((numPages*PageSize - sp) / UserThreadStackSize) - 1; 
    threadStackBitmap->Clear(i);
}

// ----------------------------------------------------------------
// 	Public access to synchronization primitives
//		Allows access to synchronization operation call without 
//		requiring direct manipulations
//
//		This part allows safe access to the thread table, synchronization
//		table and bitmap
// -----------------------------------------------------------------

int AddrSpace::LockThreadTable()
{
	if(threadTableLock == NULL){return -1;}
	threadTableLock->Acquire() ;
	return 0;
}

void AddrSpace::UnlockThreadTable() { threadTableLock->Release() ; }

void AddrSpace::WaitJoinCondition(unsigned int ID) { threadSynchTable[ID]->Wait(threadTableLock) ; }

void AddrSpace::BroadcastJoinCondition(unsigned int ID) { threadSynchTable[ID]->Broadcast(threadTableLock) ; }

void AddrSpace::WaitExitCondition()
{
	if(threadTableLock != NULL)
		threadExitCond->Wait(threadTableLock) ;
}

void AddrSpace::BroadcastExitCondition() { threadExitCond->Broadcast(threadTableLock) ; }

void AddrSpace::LockThreadStack(){ threadStackBitmapLock->Acquire() ; }

void AddrSpace::UnlockThreadStack() { threadStackBitmapLock->Release() ; }

// -------------------------------------------------------------------------
// 	ReadAtVirtual
//		Reads data from an executable file into a specified virtual address
//
//		Loads a portion of an executable file into the virtual memory space
//		reads 'numBytes' of data from 'position' in the file and writes it to
//		the given virtual address. Uses the given page table for translation
// -------------------------------------------------------------------------

static void ReadAtVirtual(OpenFile *executable, int virtualaddr, 
		int numBytes, int position, 
		TranslationEntry *pageTable, unsigned numPages) 
{
	int i ;
	char buf[numBytes] ;
	int nbBytes = executable->ReadAt(buf, numBytes, position) ;

	TranslationEntry *oldTable = machine->pageTable ;
	int oldTableSize = machine->pageTableSize ;

	machine->pageTable = pageTable ;
	machine->pageTableSize = numPages ;

	for (i = 0 ; i < nbBytes ; i ++) 
	{
		machine->WriteMem(virtualaddr + i, 1, buf[i]) ;
	}

	machine->pageTable = oldTable ;
	machine->pageTableSize = oldTableSize ;
}

// ----------------------------------------------------------------
// 	AddrSpace::FreeFrames
//		Frees the physical frames allocated to an address space, 
//		the associated page table must be valid
// -----------------------------------------------------------------
void AddrSpace::FreeFrames()
{
	for (unsigned int i = 0 ; i < numPages ; i ++) 
	{
		if (pageTable[i].valid) 
		{
			frameProvider->ReleaseFrame(pageTable[i].physicalPage) ;
		}
	}
}

// ----------------------------------------------------------------
// 	AddrSpace::IsCreated
//		Returns true if the address space was successfully created
//		else returns false
// -----------------------------------------------------------------
bool AddrSpace::IsCreated() { return isSpaceCreated ; }

// -------------------------------------------------
// 					MISCELLANOUS
// -------------------------------------------------

// -------------------------------------------------------------------------
// 	copyStringFromMachine
//		Copies a string from the simulated machine memory into the kernel
//
//		Reads the string starting from the address specified (from) and
//		copies it into a bugger in the kernel
//
//		Arguments:
//			from: the starting address of the string to copy
//			to: pointer to the destination buffer
//			size: maximum size of bytes to copy
// -------------------------------------------------------------------------
void copyStringFromMachine(int from, char *to, unsigned size) {
    unsigned i = 0;  
    int ch = 0; 
    for (; i < size - 1; i++) {
        
        if (!machine->ReadMem(from + i,1,&ch)) {
            break; 
        }
        to[i] = static_cast<char>(ch);
        if (to[i] == '\0') {
            return; 
        }
    }
    to[i] = '\0';
}

// -------------------------------------------------------------------------
// 	copyStringToMachine
//		Copies a string from the skernel into the simulated machine memory
//
//		Writes the string from a bugger in the kernel to the specified address
//
//		Arguments:
//			from: pointer to the source bugger
//			to: pointer to destination address or starting address to copy
//			size: maximum size of bytes to copy
//
//		Returns:
//			0 if the copy is successful, -1 if failed
// -------------------------------------------------------------------------
int copyStringToMachine(char *from, int to, unsigned int size) {
	unsigned int i = 0;
    
	for(; i < size - 1; i++) {
		if(!machine->WriteMem(to + i, 1, (int)from[i])){
            return -1;
        }
		if (from[i] == '\0') break;
	}

	if(!machine->WriteMem(to + i, 1, (int)'\0')) return -1;

	return i;
}