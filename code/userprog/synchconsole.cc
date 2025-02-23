#include "copyright.h"
#include "system.h"
#include "synchconsole.h"
#include "synch.h"

static Semaphore *readAvail;
static Semaphore *writeDone;
static void ReadAvail(int arg) { readAvail->V(); }
static void WriteDone(int arg) { writeDone->V(); }

static Lock *consoleWrite;      // synchronize access to the console write operation
static Lock *consoleRead;       // synchronize access to the console read operation


//--------------------------------------------------------------------------
// SynchConsole::SynchConsole
//			Constructor of the synchConsole class. Initializes the synchronized
//          console by setting coordination primitives and the underlying console
//
//			Arguments:
//				readFile: the name of the file for console input, can be NULL
//                           for direct input
//              writeFile: the name of the file for console output, can be NULL
//                              for direct output
//---------------------------------------------------------------------------

SynchConsole::SynchConsole(char *readFile, char *writeFile)
{
    readAvail = new Semaphore("read avail", 0);
    writeDone = new Semaphore("write done", 0);

    consoleWrite = new Lock("Console writing lock");
    consoleRead = new Lock("Console reading lock");

    console = new Console(readFile, writeFile, ReadAvail, WriteDone, 0); 
}

//--------------------------------------------------------------------------
// SynchConsole::~SynchConsole
//			Destructor of the synchConsole class. Cleans up all dynamically
//          allocated resources: delete the console object, delete the semaphores
//          user for read and write synchronization, delete the locks used for
//          safe access
//---------------------------------------------------------------------------

SynchConsole::~SynchConsole()
{
    delete console;
    delete writeDone;
    delete readAvail;
    delete consoleWrite;
    delete consoleRead;
}

//--------------------------------------------------------------------------
// SynchConsole::SynchPutChar
//			Writes a single character to the console
//
//          Acquires the write lock, uses the underlying console's function
//          to write the character and waits for the write operation to 
//          complete before releasing the lock
//
//			Arguments:
//				ch: the character to be written to the console
//---------------------------------------------------------------------------

void SynchConsole::SynchPutChar(const char ch)
{
    consoleWrite->Acquire();

    console->PutChar(ch); 
    writeDone->P(); 

    consoleWrite->Release();
}

//--------------------------------------------------------------------------
// SynchConsole::SynchGetChar
//			Reads a single character from the console
//
//          Acquires and releases the read lock if the boolean passed as a 
//          parameter is false since lock was already acquired in calling 
//          method. Wait that the input is available then calls underlying
//          console's Getchar to retrieve the character from the input buffer
//
//			Arguments:
//				fromGetString: a flag that indicates if the function was
//                              called from the SynchGetString method
//
//          Return:
//              a character read from the console as an integer
//---------------------------------------------------------------------------

int SynchConsole::SynchGetChar(bool fromGetString)
{
    if(! fromGetString) {
        consoleRead->Acquire();    
    }

    readAvail->P(); 
    int ch = console->GetChar();

    if(! fromGetString) {
        consoleRead->Release();
    }

    return ch;
}

//--------------------------------------------------------------------------
// SynchConsole::SynchPutString
//			Writes a string of characters to the console
//
//          Acquires the write lock, then iterates through the string as
//          long we don't hit an end of character '\0', and uses the underlying
//          console Putchar to display each character
//
//			Arguments:
//				s: an array of characters representing the string we want 
//                  to output
//---------------------------------------------------------------------------

void SynchConsole::SynchPutString(const char s[])
{
    
    if(!s || s[0] == '\0'){ return; }

    consoleWrite->Acquire();

    for(int i=0; s[i] != '\0'; i++){
        console->PutChar(s[i]);
        writeDone->P();
    }

    consoleWrite->Release();
}

//--------------------------------------------------------------------------
// SynchConsole::SynchGetString
//			Reads a string of characters from the console
//
//          Acquires the read lock, then reads one character at a time. Stops
//          reading when reaching the maximum length, and end of file, a new
//          line or a null terminator
//
//			Arguments:
//				s : a pointer to a character bugger to store the input string
//              n : the maximum number of characters to read, including the 
//                    null terminator
//---------------------------------------------------------------------------

void SynchConsole::SynchGetString(char *s, int n)
{
    if(!s) return; 

    consoleRead->Acquire();

    int i = 0;

	s[i] = SynchGetChar(true) ;

	while (i < n - 1 && s[i] != EOF && s[i] != '\n' && s[i] != '\0') 
	{
		s[++ i] = SynchGetChar(true);
	}

	s[i ++] = '\0' ;

    consoleRead->Release();

}

//--------------------------------------------------------------------------
// SynchConsole::SynchPutInt
//			Write an integer to the console
//
//          Converts the integer value into a string then outputs the sring 
//          to the console
//
//			Arguments:
//              n : the integer value to be printed
//---------------------------------------------------------------------------

void SynchConsole::SynchPutInt(int n)
{

    char int_buffer[MAX_INT_SIZE];
    snprintf(int_buffer, MAX_INT_SIZE, "%i", n);
    SynchPutString(int_buffer);

}

//--------------------------------------------------------------------------
// SynchConsole::SynchGetInt
//			Reads an integer from the console
//
//          Reads a string from the console, then parse the string to extract
//          the integer value and stores it into the memory pointed by n
//
//			Arguments:
//              n : a pointer to an integer where the input will be stored
//---------------------------------------------------------------------------

void SynchConsole::SynchGetInt(int *n)
{
    if(n == NULL) return;

    char int_buffer[MAX_INT_SIZE];

    SynchGetString(int_buffer, MAX_INT_SIZE);
    sscanf(int_buffer, "%i", n);

}






