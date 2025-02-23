#ifndef USERPROCESS_H
#define USERPROCESS_H

#include "system.h"

void waitForChildrenToExit(AddrSpace *space);
void cleanUpOnExit(int ID, AddrSpace *space);

int do_UserProcessCreate(char *filename, int arg, int returnFun);
void do_UserProcessHalt() ;
void do_UserProcessExit() ;
void do_WaitProcess(int processID);

#endif
