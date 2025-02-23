#include "shell.h"
#include "system.h"
#include "userprocess.h"

extern void StartProcess(char *file);
void Copy(const char *from, const char *to);
void Print(char *name);
void PerformanceTest();
SynchConsole *console;

/*
return if the comand goal is to halt or no
*/
int finished(char *in){
    if(in[0] == 'e' && in[1] == 'x' && in[2] == 'i' && in[3] == 't'){ return 1;}
    else if(in[0] == 'h' && in[1] == 'a' && in[2] == 'l' && in[3] == 't'){ return 1;}
    else{return 0;}
} 

void printcd(char **cd, int size){
    int j;
    for(int i = 0; i < size; i++){
        j=0;
        while(cd[i][j] != '\0'){
            console->SynchPutChar(cd[i][j]);
            j++;
        }
        console->SynchPutChar('/');
    }
}


/*
The default shell lunching if there's no user program choosen to execute
*/
void shell(SynchConsole *c){
    console = c;
    char prompt[3];
    prompt[0] = '~';
    prompt[1] = '~';
    prompt[2] = '\0';
    char *currentDir[15];
    for(int i = 0; i<15;i++){
        currentDir[i]=new char[255];
    }
    int currentDirSize = 1;
    sprintf(currentDir[0], "home");
    currentDir[0][4] = '\0';
    char in[255];
    Commande *cmd = new Commande();

    console->SynchPutChar('\n');
    console->SynchPutString("Welcome in NachOS\n");
    console->SynchPutString("if you want to know the povided services, enter help\n");
    printcd((char**)currentDir, currentDirSize);
    console->SynchPutString(prompt);

    console->SynchGetString(in, 255);
    while(finished(in) != 1){
        cmd->decode(in);
        switch (cmd->what){
            case UNKNOWN:
                console->SynchPutString("unable to recognise your commande: ");
                console->SynchPutString(in);
                console->SynchPutChar('\n');
                break;
            case HELP:
                console->SynchPutString("with the current version of NachOS, you have access to:");
                console->SynchPutString("\n");
                console->SynchPutString("   x <program> to execute a program stored on the disk\n");
                console->SynchPutString("   p <file> to print the content of a file\n");
                console->SynchPutString("   t to run the provided perfomances test\n");
                console->SynchPutString("\n");
                console->SynchPutString("   l to print the content of the current directory\n");
                console->SynchPutString("   d to print the content of the disk related to the current directory\n");
                console->SynchPutString("   cd <directory> to change the current directory\n");
                console->SynchPutString("\n");
                console->SynchPutString("   nd <name> to create a new directory\n");
                console->SynchPutString("   nf <name> <size> to create a new empty file \n");
                console->SynchPutString("   cp <UNIX name> <name> to create a copy of a file in nachos file system\n");
                console->SynchPutString("   r <file> to remove a file or a directory.\n");
                console->SynchPutString("             If file is a directory, recursively remove it content.\n");
                console->SynchPutString("\n");
                console->SynchPutString("   exit or halt to properly close Nachos and shutdown the machine\n");
                break;
            case EXEC:
                do_UserProcessCreate(cmd->txt[0], (int)&cmd->txt[1],(int) &shell);
                while(numProcess>1){
                    currentThread->Yield();
                }
                break;
            case CD:
                if(fileSystem->moveCd(cmd->txt[0]) == 1){
                    if(!strcmp(cmd->txt[0], "..") && currentDirSize >1){
                        if(currentDirSize>0){currentDirSize--;}
                    }
                    else if(strcmp(cmd->txt[0], "..") ){
                        strcpy(currentDir[currentDirSize],(char*)cmd->txt);
                        currentDirSize++;
                    }
                }
                break;
            case CP:
                Copy(cmd->txt[0], cmd->txt[1]);
                break;
            case PRINT:
                Print(cmd->txt[0]);
                break;
            case REMOVE:
                fileSystem->Remove(cmd->txt[0]);
                break;
            case LS:
                fileSystem->List();
                break;
            case D:
                fileSystem->Print();
                break;
            case TEST:
                PerformanceTest();
                break;
            case NEWDIR:
                if(cmd->txt[0][0]!= '\0'){
                    if(!fileSystem->Create(cmd->txt[0], 1, 0)){
                        console->SynchPutString("unable to create a directory here\n");
                    }
                }
                else{
                    console->SynchPutString("format: nd <name>\n");
                }
                break;
            case NEWFILE:
                if(cmd->txt[0][0]== '\0' || cmd->txt[1][0] == '\0'){
                    console->SynchPutString("format: nf <name> <size>\n");
                }
                else{
                    if(!fileSystem->Create(cmd->txt[0], atoi(cmd->txt[1]), 1)){
                        console->SynchPutString("unable to create a file here\n");
                    }
                }
                break;
            default:
                break;
        }
        printcd((char**)currentDir, currentDirSize);
        console->SynchPutString(prompt);
        console->SynchGetString(in, 255);
    }
    currentDirSize ++;
    interrupt->Halt();
}