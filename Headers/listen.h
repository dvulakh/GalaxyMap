
#include <algorithm>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>

using namespace std;

#ifndef LISTEN_H
#define LISTEN_H

#define NUM_KEY 256
#define NUM_BIT 32

class listener
{
private:

	VOID (*press)();
	bool caps_lock;
	void add(int);

public:

	int kb[NUM_KEY / NUM_BIT + 1];
	string received;
	bool running;

	listener(VOID(*)());
	listener();
	
	void clear();
	void run();

};

#endif

