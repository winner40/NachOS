#include "syscall.h"

void printString(char* m) {
    PutString("Thread printing string: \n");
    PutString(m);
    PutChar('\n');
    UserThreadExit();
}

int main(){
    PutString("In main function \n");
    char* message = "Testing thread string!";
    int fnargs2 = UserThreadCreate((void*)printString, message);
    if( fnargs2 < 0){
        PutString("Failed to create thread(string) \n");
    }else{
        PutString("Thread was created!!\n");
    }
    PutString("Back in main function \n");
    UserThreadJoin(fnargs2);

    return(0);
}