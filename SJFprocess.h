#ifndef SJFPROCESS_H
#define SJFPROCESS_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <math.h>
#include <queue>
#include <stdlib.h>
#include <list>
#include <utility>
#include <iostream>
#include <vector>

using namespace std;

extern double next_exp(int);


extern double LAMBDA;
extern double ALPHA;


//ID to char
extern char idtoc(int id );





class SJFProcess {
public:
	int ID;
	int arrivalTime;
	list<int> bursts;
	bool arrived = false;
	bool isCPUBound = false;

	int priority = 0;
	int completedCPUBursts = 0;
	int completedIOBursts = 0;
	int totalCPUBursts;


	int time_using_cpu = 0;


	int total_turnaround_time = 0;
	int total_wait_time = 0;





	int last_burst_time_using_cpu = 0;



	SJFProcess(int id, int arrTime, int numBursts, bool CPUBound) {
		ID = id;
		arrivalTime = arrTime;
		totalCPUBursts = numBursts;

		for(int j=0;j<numBursts;j++){
			int x = next_exp(1) * (CPUBound ? 4 : 1);
			bursts.push_back(x);
            if (j!=numBursts-1)
                bursts.push_back(next_exp(1)*10 / (CPUBound ? 8 : 1));
        }
        isCPUBound = CPUBound;
        priority = ceil(1/LAMBDA);
	}

	void elapseTime(int t) {
		if (arrived) {
			bursts.front() -= t;
			if (bursts.front() == 0) {
				bursts.pop_front();
				if (completedCPUBursts > completedIOBursts) {
					completedIOBursts++;
				} else {
					completedCPUBursts++;
				}
			}
		} else {
			arrivalTime -= t;
			if (arrivalTime == 0) {
				arrived = true;
			}
		}
	}
	void elapseTurnaroundTime(int t) {
		total_turnaround_time+=t;
	}
	void elapseWaitTime(int t) {
		total_wait_time+=t;
	}

	bool shouldTerminate() {
		return completedCPUBursts == completedIOBursts + 1 && completedCPUBursts == totalCPUBursts;
	}

	void incTimeUsingCPU(int t) {
		time_using_cpu += t;
		last_burst_time_using_cpu += t;
	}

	int nextFinish() {
		return bursts.front();
	}


	int getPriority() {
		return priority;
	}


	void updatePriority() {
		priority = ceil(ALPHA * last_burst_time_using_cpu + ( 1 - ALPHA ) * priority);
		// printf("last burst: %d\n",last_burst_time_using_cpu);
		last_burst_time_using_cpu = 0;
	}
private:


};
class SJFCompare {
public:
    bool operator()(SJFProcess* l, SJFProcess* r) {
        if (l->getPriority() == r->getPriority())
            return l->ID > r->ID;
        
        return l->getPriority() > r->getPriority();
    }
};
class SJFArrivalTimeCompare {
public:
    bool operator()(SJFProcess* l, SJFProcess* r) {
        if (l->arrivalTime == r->arrivalTime)
            return l->ID > r->ID;
        
        return l->arrivalTime > r->arrivalTime;
    }
};
class SJFIOBurstTimeCompare {
public:
    bool operator()(SJFProcess* l, SJFProcess* r) {
        if (l->nextFinish() == r->nextFinish())
            return l->ID > r->ID;
        
        return l->nextFinish() > r->nextFinish();
    }
};


#endif