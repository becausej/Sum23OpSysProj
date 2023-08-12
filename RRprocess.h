#ifndef RRPROCESS_H
#define RRPROCESS_H
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

extern float next_exp(int);


//ID to char
extern char idtoc(int id );





class RRProcess {
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


	int tempburst = 0;


	RRProcess(int id, int arrTime, int numBursts, int CPUBound) {
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
        tempburst = bursts.front();
	}

	void elapseTime(int t, int flag) {
		if (arrived) {
			bursts.front() -= t;
			if (bursts.front() == 0 && (flag == 0 || flag == 3)) {
				bursts.pop_front();
				tempburst = bursts.front();
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

	int nextFinish() {
		return bursts.front();
	}

	void incNextFinish(int t) {
		bursts.front() += t;
	}

	int getPriority() {
		return priority;
	}
private:


};
class RRCompare {
public:
    bool operator()(RRProcess* l, RRProcess* r) {
        // int ladj = l->tempburst - l->nextFinish();
        // int radj = r->tempburst - r->nextFinish();
        // if (l->getPriority()-ladj == r->getPriority()-radj)
        //     return l->ID > r->ID;
        
        
        return l->getPriority() > r->getPriority();
    }
};
class RRArrivalTimeCompare {
public:
    bool operator()(RRProcess* l, RRProcess* r) {
        if (l->arrivalTime == r->arrivalTime)
            return l->ID > r->ID;
        
        return l->arrivalTime > r->arrivalTime;
    }
};
class RRIOBurstTimeCompare {
public:
    bool operator()(RRProcess* l, RRProcess* r) {
        if (l->nextFinish() == r->nextFinish())
            return l->ID > r->ID;
        
        return l->nextFinish() > r->nextFinish();
    }
};


#endif