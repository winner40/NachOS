#include "syscall.h"

void TestString(){
    int string_size = 255;
    char input_string[string_size];

    PutString("Enter your string here: \n");
    GetString(input_string, string_size);
    PutString("\n");
    PutString("You entered the following string: \n");
    PutString(input_string);
    PutString("\n");
    PutString("\n");
}

int main() {

    PutString("Testing function call Putstring. \n");
    TestString();
  
    return 0;
}
