// exception.cc
//      Entry point into the Nachos kernel from user programs.
//      There are two kinds of things that can cause control to
//      transfer back to here from user code:
//
//      syscall -- The user code explicitly requests to call a procedure
//      in the Nachos kernel.  Right now, the only function we support is
//      "Halt".
//
//      exceptions -- The user code does something that the CPU can't handle.
//      For instance, accessing memory that doesn't exist, arithmetic errors,
//      etc.
//
//      Interrupts (which can also cause control to transfer from user
//      code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "syscall.h"
#include "system.h"
#include "userthread.h"
#include "userSem.h"
#include "userprocess.h"
#include "filesys.h"

//----------------------------------------------------------------------
// UpdatePC : Increments the Program Counter register in order to resume
// the user program immediately after the "syscall" instruction.
//----------------------------------------------------------------------
static void UpdatePC() {
    int pc = machine->ReadRegister(PCReg);
    machine->WriteRegister(PrevPCReg, pc);
    pc = machine->ReadRegister(NextPCReg);
    machine->WriteRegister(PCReg, pc);
    pc += 4;
    machine->WriteRegister(NextPCReg, pc);
}

//----------------------------------------------------------------------
// ExceptionHandler
//      Entry point into the Nachos kernel.  Called when a user program
//      is executing, and either does a syscall, or generates an addressing
//      or arithmetic exception.
//
//      For system calls, the following is the calling convention:
//
//      system call code -- r2
//              arg1 -- r4
//              arg2 -- r5
//              arg3 -- r6
//              arg4 -- r7
//
//      The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//      "which" is the kind of exception.  The list of possible exceptions
//      are in machine.h.
//----------------------------------------------------------------------

int type;
int arg1;
int arg2;
int arg3;
int arg4;
Semaphore *tmp;


void ExceptionHandler(ExceptionType which) {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    type = machine->ReadRegister(2);
    arg1 = machine->ReadRegister(4);
    arg2 = machine->ReadRegister(5);
    arg3 = machine->ReadRegister(6);
    arg4 = machine->ReadRegister(7);

    if (which == SyscallException) {

        switch (type)
        {
            case SC_Halt:
                DEBUG('a', "Shutdown, initiated by user program.\n");

                do_UserProcessHalt();
                break;

            case SC_GetChar:
                machine->WriteRegister(2, synchConsole->SynchGetChar(false));
                break;

            case SC_PutChar:
                synchConsole->SynchPutChar((char)arg1);
                break;

            case SC_PutInt:
                synchConsole->SynchPutInt((int)arg1);
                break;
            
            case SC_GetInt: {
                // IF ERROR on getting int coming back here
                int to = (int) arg1;
                int tmp_val;
                // int r = (int **)addr;
                synchConsole->SynchGetInt(&tmp_val);
                machine->WriteMem(to, sizeof(int), tmp_val);
                machine->WriteRegister(2, 0);
                break;
            }
            
            case SC_GetString: {
                int to = arg1;
                unsigned string_size = arg2;
                char *string_buffer = new char[string_size];
                
                synchConsole->SynchGetString(string_buffer, string_size);
                copyStringToMachine(string_buffer, to, string_size);

                delete[] string_buffer;
                break;
            }
            
            case SC_PutString: {
                char *string_buffer = new char[MAX_STRING_SIZE];
                int from = arg1;
                copyStringFromMachine(from, string_buffer, MAX_STRING_SIZE);
                synchConsole->SynchPutString(string_buffer);
                delete[] string_buffer;
                break;
            }

            case SC_ThreadCreate: {
                DEBUG('a', "Creation of a new user thread, initiated by user program.\n");
                int cres = do_UserThreadCreate(arg1,arg2,arg3);
                machine->WriteRegister(2, cres);
                break;
            }
            case SC_ThreadExit:
                DEBUG('a', "Termination of a user thread, initiated by user program.\n");
                do_UserThreadExit();
                break;

            case SC_ThreadJoin:
                DEBUG('a', "Joining a user thread, initiated by user program.\n");
                do_UserThreadJoin((int)arg1);
                break;

            case SC_SemInit:
                DEBUG('a', "Creating a new user semaphore.\n");
                tmp = SemInit(arg1);
                machine->WriteRegister(2, (int) tmp);
                break;

            case SC_SemP:
                DEBUG('a', "UserSem->P().\n");
                SemP((Semaphore *)arg1);
                break;

            case SC_SemV:
                DEBUG('a', "UserSem->V().\n");
                SemV((Semaphore *)arg1);
                break;

            case SC_Exit:
                if(arg1 == 0){
                    DEBUG('a', "User program exiting normally\n");
                }
                else{
                    printf("User program exiting with an error: %d\n", arg1);
                }
                do_UserProcessExit();
                break;
            
            case SC_ForkExec: {
                char filename[MAX_FILENAME];
                copyStringFromMachine(arg1, filename, MAX_FILENAME);
                int res = do_UserProcessCreate(filename, arg2, arg3);
                machine->WriteRegister(2, res);
                break;
            }

            case SC_WaitProcess: {
                if(arg1 > TEMP_MAXPROC_NUMBER || arg1 < 0 || arg1 == currentThread->space->processID){
                    fprintf(stderr, "Error in Exception: Got a wrong process ID in WaitProcess: %d and current: %d\n", arg1, currentThread->space->processID);
                    machine->WriteRegister(2, -1);
                }

                do_WaitProcess(arg1);
                machine->WriteRegister(2, 1);
                break;
            }

            case SC_GetProcessID: 
                machine->WriteRegister(2, currentThread->space->processID);
                break;

            case SC_Create: 
                char filename[MAX_FILENAME];
                copyStringFromMachine(arg1, filename, MAX_FILENAME);
                machine->WriteRegister(2, fileSystem->Create(filename, arg2, 1));
                break;

            case SC_Open: 
                char filenameOpen[MAX_FILENAME];
                copyStringFromMachine(arg1, filenameOpen, MAX_FILENAME);
                machine->WriteRegister(2, fileSystem->do_userOpen(filename));
                break;

            case SC_Write: 
                char filenameWrite[MAX_FILENAME];
                copyStringFromMachine(arg1, filenameWrite, MAX_FILENAME);
                if(arg3 == 0){synchConsole->SynchPutString(filenameWrite);}
                if(arg3 == 1){UpdatePC();return;}
                fileSystem->do_userWrite(filenameWrite,arg2, arg3);
                break;

            case SC_Read: 
                int nbRead;
                char filenameRead[MAX_FILENAME];                
                if(arg3 == 1){synchConsole->SynchGetString(filenameRead, arg2); nbRead = arg2;}
                if(arg3 == 0){UpdatePC();return;}
                nbRead = fileSystem->do_userRead(filenameRead,arg2, arg3);
                copyStringToMachine(filenameRead, arg1, arg2);
                machine->WriteRegister(2, nbRead);
                break;

            case SC_Close:
                if(arg1 >=2){
                    fileSystem->do_userClose(arg1);
                }
                break;

            default:
                printf("Unexpected!!!!!!!!!!!!! user mode exception %d %d\n", which, type);
                // ASSERT(FALSE);
                arg1++;
                arg2++;
                arg3++;
                arg4++;
                break;
        }
    }  else if( which == PageFaultException){
        fprintf(stderr, "Error in Exception: Page Fault Error. \n");
    }
     (void)interrupt->SetLevel(oldLevel);

    // LB: Do not forget to increment the pc before returning!
    UpdatePC();
    // End of addition
}
