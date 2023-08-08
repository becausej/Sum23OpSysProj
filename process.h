#ifndef PROCESS_H
#define PROCESS_H
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


//ID to char
char idtoc(int id ) {
	return (char) id + 65;
}





class Process {
public:
	int ID;
	int arrivalTime;
	list<int> bursts;
	bool arrived = false;
	bool done = false;

	int priority = 0;
	int completedCPUBursts = 0;
	int completedIOBursts = 0;
	int totalCPUBursts;

	Process(int id, int arrTime, int numBursts, int CPUBound) {
		ID = id;
		arrivalTime = arrTime;
		totalCPUBursts = numBursts;

		for(int j=0;j<numBursts;j++){
			bursts.push_back(next_exp(1) * (CPUBound ? 4 : 1));
            if (j!=numBursts-1)
                bursts.push_back(next_exp(1)*10 / (CPUBound ? 8 : 1));
        }


        // printf("ID: %c\n", idtoc(ID));
        // printf("nuMBursts: %d\n", totalCPUBursts);
        // printf("bursts size: %ld\n",bursts.size());
		// printf("list: ");
		// int counter = 0;
		// for (auto i: bursts) {
		// 	printf("%d ",i);
		// 	counter++;
		// }
		// printf("\n%d\n\n\n\n",counter);
	}


	// int burstTime() {
	// 	return bursts.front();
	// }

	void elapseTime(int t) {
		// printf("Process elapseTime:\n");
		if (arrived) {
			// printf("1\n");
			// printf("list: ");
			// for (auto i: bursts) {
			// 	printf("%d ",i);
			// }
			// printf("\n");
			// bursts.pop_front();
			bursts.front() -= t;
			// printf("2\n");
			if (bursts.front() == 0) {
				// printf("2.1\n");
				// printf("list: ");
				// for (auto i: bursts) {
				// 	printf("%d ",i);
				// }
				// printf("\n");
				bursts.pop_front();
				// printf("COMPLETED CPU BURSTS 2.2\n");
				if (completedCPUBursts > completedIOBursts) {
					completedIOBursts++;
				} else {
					completedCPUBursts++;
				}

				// if (completedCPUBursts == completedIOBursts  && completedCPUBursts == totalCPUBursts) {
				// 	printf("Process %c done\n",idtoc(ID));
				// 	done = true;
				// }
			}
			// printf("3\n");
		} else {
			// printf("2\n");
			// printf("%c arrivalTime: %d\n",(char)(ID+65),arrivalTime);
			arrivalTime -= t;
			if (arrivalTime == 0) {
				arrived = true;
			}
		}
		// printf("Process end elapseTime:\n");
	}

	bool shouldTerminate() {
		return completedCPUBursts == completedIOBursts + 1 && completedCPUBursts == totalCPUBursts;
	}

	int nextFinish() {
		return bursts.front();
	}

	void incNextFinish(int inc) {
		bursts.front() += inc;
	}
private:


};
class Compare {
public:
    bool operator()(Process* l, Process* r) {
        if (l->priority == r->priority)
            return l->ID > r->ID;
        
        return l->priority > r->priority;
    }
};
class ArrivalTimeCompare {
public:
    bool operator()(Process* l, Process* r) {
        if (l->arrivalTime == r->arrivalTime)
            return l->ID > r->ID;
        
        return l->arrivalTime > r->arrivalTime;
    }
};
class IOBurstTimeCompare {
public:
    bool operator()(Process* l, Process* r) {
        if (l->nextFinish() == r->nextFinish())
            return l->ID > r->ID;
        
        return l->nextFinish() > r->nextFinish();
    }
};


#endif