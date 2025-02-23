#ifndef THREADPARAMS_H
#define THREADPARAMS_H

struct ThreadParams {
    int threadFunction ; 		// function that the thread will execute
	int functionArgs ;			// arguments for the thread function
	int returnFunction ;		// function to be executed after thread has completed exection
	bool shouldSetSpace ;		// inidicates if an address space should be set for the thread

    ThreadParams(int _fun, int _arg, int _returnFun, bool _setSpace)
        : threadFunction(_fun), functionArgs(_arg), returnFunction(_returnFun), shouldSetSpace(_setSpace) {}
};

#endif
