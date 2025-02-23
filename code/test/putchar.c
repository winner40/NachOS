#include "syscall.h"

void print(char c, int n) {
    int i;
    for (i = 0; i < n; i++) {
        PutChar(c + i);
    }
    PutChar('\n');  
}

void TestChar() {
    //Copying Test
    char* message = "Testing system call for characters: Hey there!!!!";
    int i = 0;

    while (message[i] != '\0'){
        PutChar(message[i]); // printing each char of the message
        i++;
    }
    PutChar('\n');

    //Reading From Input Test
    char c = GetChar();
    while(c != 'x'){
        PutChar(c);
        c = GetChar();
    }
    PutChar('\n');
}

int main() {
    print('a', 4);
    // TestChar();
    Halt();         
    return 0;
}
