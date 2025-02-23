#ifndef USERTHREAD_H
#define USERTHREAD_H

extern int do_UserThreadCreate(int f, int arg, int returnFun);

extern void do_UserThreadExit();

extern int do_UserThreadJoin(int ID);

extern int do_UserThreadGetID();

#endif
