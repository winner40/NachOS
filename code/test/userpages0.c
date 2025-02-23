#include "syscall.h"

void printChars(char* s) {
	int nb = 10;
	int i;

	for(i=0;i<nb;i++) {
		PutString((char*)s);
        for(int j=0; j < 10000; j++){
            // simulate work
        }
    }
	
	UserThreadExit();
}

int main () {	
	PutString("Starting main in userpages0.\n");

	void* f = printChars;
    int nbThreads = 2;
    int currentIdx = 0;
    int threadsToWait[nbThreads];

    int th1 = UserThreadCreate(f,"A");
    int th2 = UserThreadCreate(f,"B");

    threadsToWait[currentIdx++] = th1;
    threadsToWait[currentIdx++] = th2;

    for(int i = 0; i < nbThreads; i++){
        UserThreadJoin(threadsToWait[i]);
    }

    PutString("\nEnd of userpages0 main.\n");

    return 0;
}