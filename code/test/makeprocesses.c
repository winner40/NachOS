#include "syscall.h"

int main()
{
   PutString("Main Process 1\n");
//    int p1 = ForkExec("./makethreads");
   int p1 = ForkExec("./makethreads");
   int p2 = ForkExec("./userpages0");
//    WaitProcess(p1);

   PutString("Main Process 2\n");

   WaitProcess(p1);
   WaitProcess(p2);

   return 0;
}