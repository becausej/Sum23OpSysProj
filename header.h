#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

double LAMBDA;
int UPPER_BOUND;
double next_exp(int mode);

enum STATE {
	RUNNING = 0,
	READY = 1,
	WAITING = 2
};




#endif // HEADER_H