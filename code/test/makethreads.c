#include "syscall.h"

void printThread1() {
    PutString("Starting work in thread 1. \n");
    for(int i = 0; i < 5; i++){
        PutString("Thread 1 iteration nb: ");
        PutInt(i);
        PutChar('\n');
    }
    PutString("Finishing work in thread 1. \n");
    UserThreadExit();
}

void printThread2() {
    PutString("Starting work in thread 2. \n");
    for(int i = 0; i < 7; i++){
        PutString("Thread 2 iteration nb: ");
        PutInt(i);
        PutChar('\n');
    }
    PutString("Finishing work in thread 2. \n");
    UserThreadExit();
}

void printThread3() {
    PutString("Starting long work in thread 3. \n");
    int sum = 0;
    for(int i = 0; i < 1000; i++){
        sum += i;
    }
    PutString("Summation done in thread 3! Res: ");
    PutInt(sum);
    PutChar('\n');
    PutString("Finishing work in thread 3. \n");
    UserThreadExit();
}




int main() {

    int nbThreads = 5;
    int currentIdx = 0;
    int threadsToWait[nbThreads];

    PutString("Starting work in makethreads. \n");
    
    int t1 = UserThreadCreate((void*)printThread1, 0);
    int t2 = UserThreadCreate((void*)printThread2, 0);
    int t3 = UserThreadCreate((void*)printThread3, 0);
    // int t4 = UserThreadCreate((void*)printThread3, 0);

    threadsToWait[currentIdx++] = t1;
    threadsToWait[currentIdx++] = t2;
    threadsToWait[currentIdx++] = t3;
    // threadsToWait[currentIdx++] = t4;
    
    for(int i = 0; i < currentIdx; i++){
        UserThreadJoin(threadsToWait[i]);
    }

    PutString("Ending main program.");   
    return 0;
}
