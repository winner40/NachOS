#include "syscall.h"
sem_t *sem;
int sum;

void t1(){
    int j = 0;
    for(j = 0; j < 10; j++){
        SemP(sem);
        sum ++;
        PutString("I'm 1 and sum = ");
        PutInt(sum);
        PutChar('\n');
        SemV(sem);
    }
    PutString("t1 finish\n");
    UserThreadExit();
}

void t2(){
    int i = 0;
    for(i = 0; i < 10; i++){
        SemP(sem);
        PutString("I'm 2 and sum = ");
        sum ++;
        PutInt(sum);
        PutChar('\n');
        SemV(sem);
    }
    PutString("t2 finish\n");
    UserThreadExit();
}

int main(){
    
    sem = SemInit(1);
    sum = 0;
    int one = UserThreadCreate((void *)t1, 0);
    int two = UserThreadCreate((void *)t2, 0);

    UserThreadJoin(one);
    UserThreadJoin(two);
    return 0;
}