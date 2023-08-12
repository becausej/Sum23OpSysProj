#ifndef SRTPROCESS_H
#define SRTPROCESS_H
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


extern float LAMBDA;
extern float ALPHA;



//ID to char
extern char idtoc(int id );





class SRTProcess {
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
	bool got_preempted = false;




	int last_burst_time_using_cpu = 0;



	SRTProcess(int id, int arrTime, int numBursts, bool CPUBound) {
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
        if (LAMBDA != 0)
        	priority = ceil(1/LAMBDA);
        else
        	priority = INT_MAX;
        tempburst = bursts.front();
	}

	void elapseTime(int t, int flag) {
		if (arrived) {
			bursts.front() -= t;
			if (bursts.size() != 0)
				if (bursts.front() == 0 &&
					((flag == 0 && completedCPUBursts == completedIOBursts) ||
						(flag == 3 && completedCPUBursts > completedIOBursts))) {
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
		// priority = ceil(ALPHA * last_burst_time_using_cpu + ( 1 - ALPHA ) * priority);
		priority = ceil( ALPHA * last_burst_time_using_cpu + ( 1.0 - ALPHA ) * priority);
		last_burst_time_using_cpu = 0;
	}

	int estTimeRemaining() {
		return priority - (tempburst - bursts.front());
	}
private:


};
class SRTCompare {
public:
    bool operator()(SRTProcess* l, SRTProcess* r) {
        // if (l->getPriority() == r->getPriority())
        //     return l->ID > r->ID;
        
        // return l->getPriority() > r->getPriority();
        int ladj = l->tempburst - l->nextFinish();
        int radj = r->tempburst - r->nextFinish();
        if (l->getPriority()-ladj == r->getPriority()-radj)
            return l->ID > r->ID;
        
        
        return l->getPriority() - ladj > r->getPriority() - radj;
    }
};
class SRTArrivalTimeCompare {
public:
    bool operator()(SRTProcess* l, SRTProcess* r) {
        if (l->arrivalTime == r->arrivalTime)
            return l->ID > r->ID;
        
        return l->arrivalTime > r->arrivalTime;
    }
};
class SRTIOBurstTimeCompare {
public:
    bool operator()(SRTProcess* l, SRTProcess* r) {
        if (l->nextFinish() == r->nextFinish())
            return l->ID > r->ID;
        
        return l->nextFinish() > r->nextFinish();
    }
};


#endif