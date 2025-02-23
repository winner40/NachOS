#include "syscall.h"

void TestInt() {
    int random_int = 2147483645;
    int input_int;
    PutString("Testing system call for integers. \n");

    //testing put
    PutInt(random_int);
    PutChar('\n');

    //testing get
    PutString("Enter an integer here: \n");
    GetInt(&input_int);
    PutString("You entered: \n");
    PutInt(input_int);
    PutChar('\n');
}

int main() {

    //Call function test syscall 
    TestInt();
    //Halt();
  
    return 0;
}
