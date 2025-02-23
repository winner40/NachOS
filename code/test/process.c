#include "syscall.h"

int main(){
    ForkExec("./1");
    ForkExec("./2");
    PutString("I forked, now I finish\n");
    Exit(0);
}