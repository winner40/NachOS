// progtest.cc
//      Test routines for demonstrating that Nachos can load
//      a user program and execute it.
//
//      Also, routines for testing the Console hardware device.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "addrspace.h"
#include "console.h"
#include "copyright.h"
#include "synch.h"
#include "system.h"
#include "synchconsole.h"

//----------------------------------------------------------------------
// StartProcess
//      Run a user program.  Open the executable, load it into
//      memory, and jump to it.
//----------------------------------------------------------------------

void StartProcess(char *filename) {
    OpenFile *executable = fileSystem->Open(filename);
    AddrSpace *space;

    if (executable == NULL) {
        printf("Unable to open file %s\n", filename);
        return;
    }
    space = new AddrSpace(executable);
    currentThread->space = space;

    delete executable; // close file

    space->InitRegisters(); // set the initial register values
    space->RestoreState();  // load page table register

    machine->Run(); // jump to the user progam
    ASSERT(FALSE);  // machine->Run never returns;
    // the address space exits
    // by doing the syscall "exit"
}

// Data structures needed for the console test.  Threads making
// I/O requests wait on a Semaphore to delay until the I/O completes.

static Console *console;
static SynchConsole *synchconsole;
static Semaphore *readAvail;
static Semaphore *writeDone;


//----------------------------------------------------------------------
// ConsoleInterruptHandlers
//      Wake up the thread that requested the I/O.
//----------------------------------------------------------------------

static void ReadAvail(int arg) { readAvail->V(); }
static void WriteDone(int arg) { writeDone->V(); }

//----------------------------------------------------------------------
// ConsoleTest
//      Test the console by echoing characters typed at the input onto
//      the output.  Stop when the user types a 'q'.
//----------------------------------------------------------------------

void ConsoleTest(char *in, char *out) {
    char ch;

    console = new Console(in, out, ReadAvail, WriteDone, 0);
    readAvail = new Semaphore("read avail", 0);
    writeDone = new Semaphore("write done", 0);

    char EOFflag = 0;

    for (;;) {

        readAvail->P(); // wait for character to arrive
        ch = console->GetChar();
        if (ch == EOF)
            return;

        console->PutChar('<'); // echo it!
        writeDone->P();       // wait for write to finish

        while(ch != '\n'){
            console->PutChar(ch); // echo it!
            writeDone->P();       // wait for write to finish

            readAvail->P(); // wait for character to arrive
            ch = console->GetChar();

            if (ch == EOF){
                ch = '\n';
                EOFflag = 1;
            }
        }

        console->PutChar('>'); // echo it!
        writeDone->P();       // wait for write to finish

        console->PutChar('\n'); // echo it!
        writeDone->P();       // wait for write to finish
    }

    if(EOFflag)
        return;

    
}

// working version of synch console test
void SynchConsoleTest(char *in, char *out) {
    char ch;
    char EOFflag = 0;
    char input_buffer[255]; //temp buffer for a single line
    int idx = 0;

    synchconsole = new SynchConsole(in, out);

    while (true){
        ch = synchconsole->SynchGetChar(false);

        if(ch == EOF) {
            EOFflag = 1;
            break;
        }

        //putting everything to console once we hit an enter
        if(ch == '\n') {
            synchconsole->SynchPutChar('<');
            for(int i = 0; i < idx; ++i){
                synchconsole->SynchPutChar(input_buffer[i]);
            }
            synchconsole->SynchPutChar('>');
            synchconsole->SynchPutChar('\n');

            idx = 0; //resetting the index
        } else {
            // store every input inside a buffer first
            if( idx < 255){
                input_buffer[idx++] = ch;
            }
        }


    }

    if(EOFflag){
        return;
    }
}

// synch console test for testing strings and integers
void SynchConsoleTest_SI(char *in, char *out) {
    
    synchconsole = new SynchConsole(in, out);

    // buffer to test strings -  need?
    char string_bugger[255];
    int number; //will the store the integer using this one here

    // Test SynchGetString and SynchPutString
    synchconsole->SynchPutString("Testing SynchGetString  and SynchPutString. \n");
    synchconsole->SynchPutString("Let's try with a string up to xxx characters, IDK: \n");
    synchconsole->SynchGetString(string_bugger, sizeof(string_bugger));
    synchconsole->SynchPutString("You entered : "); // echoing the whole world
    synchconsole->SynchPutString(string_bugger);
    synchconsole->SynchPutString("\n");

    // //Testing SynchGetInt and SynchPutInt
    synchconsole->SynchPutString("Testing SynchGetInt  and SynchPutInt : \n");
    synchconsole->SynchPutString("Let's try with an integer up to xxx digits, IDK: \n");
    synchconsole->SynchGetInt(&number);
    synchconsole->SynchPutString("You entered : "); // echoing the whole world
    synchconsole->SynchPutInt(number);
    synchconsole->SynchPutString("\n");

}

