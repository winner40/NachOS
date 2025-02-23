#include "synchconsole.h"

void shell(SynchConsole *console);

#define UNKNOWN 0
#define EXEC 1
#define CD 2
#define HELP 3
#define CP 4
#define PRINT 5
#define REMOVE 6
#define LS 7
#define D 8
#define TEST 9
#define NEWDIR 10
#define NEWFILE 11
#define CURENTSECTOR 12

class Commande{
    public: 
        char txt[2][255];
        int what;
        Commande(){}
        void decode(char *in){
            what = UNKNOWN;
            int i = 2;
            int previ = 0;
            int j = 0;
            switch(in[0]){
            case 'x':
                what = EXEC;
                while(i < 254 && in[i] != '\0'){ 
                    if(in[i] == ' ' && j == 0){txt[j][i] = '\0'; j = 1; i++; previ = 0;}
                    txt[j][previ] = in[i]; i++; previ++;
                    }
                    txt[j][previ] = '\0';

                break;
            case 'h':
                if(in[1] == 'e' && in[2] == 'l' && in[3] == 'p'){ 
                    what = HELP;}
                break;
            case 'c':
                if(in[1] == 'p'){
                    what = CP;
                    i++;
                    while(i < 254 && in[i] != '\0'){ 
                        if(in[i] == ' ' && j == 0){txt[j][i] = '\0'; j = 1; i++; previ = 0;}
                        txt[j][previ] = in[i]; i++; previ++;
                        }
                    txt[j][previ] = '\0';
                }
                else if(in[1] == 'd'){
                    what = CD;
                    i++;
                    while(i < 254 && in[i] != '\0'){
                    txt[j][previ] = in[i]; i++; previ++;
                    }
                    txt[j][previ] = '\0';
                }
                break;
            case 'p':
                what = PRINT;
                break;
            case 'r':
                what = REMOVE;
                while(i < 254 && in[i] != '\0'){
                    txt[j][previ] = in[i]; i++; previ++;
                    }
                    txt[j][previ] = '\0';
                break;
            case 'l':
                what = LS;
                break;
            case 'd':
                what = D;
                break;
            case 't':
                what = TEST;
                break;
            case 'n':
                i++;
                if(in[1] == 'd'){
                    what = NEWDIR;
                    while(i < 254 && in[i] != '\0'){
                        txt[j][previ] = in[i]; i++; previ++;
                    }
                    txt[j][previ] = '\0';
                }
                if(in[1] == 'f'){
                    what = NEWFILE;
                    while(i < 254 && in[i] != '\0'){
                    if(in[i] == ' ' && j == 0){txt[j][i] = '\0'; j = 1; i++; previ = 0;}
                        txt[j][previ] = in[i]; i++; previ++;
                    }
                    txt[j][previ] = '\0';
                }
                break;
            default:
                break;
            }
            if(j==0){
                txt[1][0] = '\0';
            }

        }
};