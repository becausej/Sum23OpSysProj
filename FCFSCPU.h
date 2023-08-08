#ifndef FCFSCPU_H
#define FCFSCPU_H

#include "process.h"
#include <math.h>
#include <climits>

unsigned long CUTOFF = 1000000;

class FCFSCPU {
public:
	vector<Process*> processes;
	Process* inCPU = NULL;
	priority_queue<Process*,vector<Process*>,ArrivalTimeCompare> incoming;
	priority_queue<Process*,vector<Process*>,Compare> readyQ;
	priority_queue<Process*,vector<Process*>,IOBurstTimeCompare> IOBursts;
	int ctxSwitchTime;

	unsigned long time=0;
	int CPUOpenIn = INT_MAX;
	int readyQCounter = 0;

	FCFSCPU(vector<Process*> procs, int switchTime) {
		ctxSwitchTime = switchTime;
		for (size_t i = 0; i < procs.size(); i++) {
			incoming.push(procs[i]);
		}

		processes = procs;
	}

	int getNextEvent() {
		// printf("1.1\n");
		int CPUdone = INT_MAX;
		if (inCPU != NULL)
			CPUdone = inCPU->nextFinish();
		
		int next_ready = INT_MAX;
		if (inCPU == NULL && !readyQ.empty()) {
			if (CPUOpenIn == INT_MAX) {
				CPUOpenIn = ctxSwitchTime / 2;
			}
			next_ready = CPUOpenIn;
		} else {
			CPUOpenIn = INT_MAX;
		}


		int nextIOFinish = INT_MAX;
		if (!IOBursts.empty()) {
			nextIOFinish = IOBursts.top()->nextFinish();
		}

		int next_incoming = INT_MAX;
		if (!incoming.empty()) {
			next_incoming = incoming.top()->arrivalTime;
		}

		
		int min = CPUdone;
		int flag = 0;
		if (next_ready < min) {
			min = next_ready;
			flag = 1;
		}

		if (nextIOFinish < min) {
			min = nextIOFinish;
			flag = 2;
		}

		if (next_incoming < min) {
			min = next_incoming;
			flag = 3;
		}

		return flag;

	}


	void run() {
		printf("time %ldms: Simulator started for FCFS ",time);
		printReady();



		while (!readyQ.empty() || !incoming.empty() || !IOBursts.empty() || inCPU != NULL ) {
			int flag = getNextEvent();

			// printf("past event\n");
			if (flag == 0) {
				// printf("flag 0\n");
				Process* p = inCPU;
				int t = p->nextFinish();
				elapseTime(t);
				inCPU = NULL;

				if (p->shouldTerminate()) {
					printTime();
					printf("Process %c terminated ", idtoc(p->ID));
					printReady();
				} else {
					// printf("cpu finished: totalCPU CPUcomp IOcomp left ==== %d %d %d %d\n",p->totalCPUBursts, p->completedCPUBursts, p->completedIOBursts, p->totalCPUBursts - p->completedCPUBursts);
					IOBursts.push(p);
					if (time < CUTOFF) printTime();
					if (time < CUTOFF) printf("Process %c completed a CPU burst; %d bursts to go ", idtoc(p->ID), p->totalCPUBursts - p->completedCPUBursts);
					if (time < CUTOFF) printReady();
					if (time < CUTOFF) printTime();
					if (time < CUTOFF) printf("Process %c switching out of CPU; blocking on I/O until time %ldms ", idtoc(p->ID), p->nextFinish() + time + ctxSwitchTime/2);
					p->incNextFinish(ctxSwitchTime/2);
					if (time < CUTOFF) printReady();
				}
				CPUOpenIn = ctxSwitchTime;
			} else if (flag == 1) {
				// printf("flag 1\n");
				Process* p = readyQ.top();
				int t = CPUOpenIn;
				elapseTime(t);
				readyQ.pop();
				if (time < CUTOFF) printTime();
				inCPU = p;
				inCPU->priority = 0;
				if (time < CUTOFF) printf("Process %c started using the CPU for %dms burst; ", idtoc(inCPU->ID), inCPU->nextFinish());
				if (time < CUTOFF) printReady();
			} else if (flag == 2) {
				Process* p = IOBursts.top();
				int t = p->nextFinish();
				IOBursts.pop();
				elapseTime(t);
				// printf("flag 2.4\n");
				// printf("p ID: %c\n",(char) (p->ID + 65));
				// printf("p num bursts: %d\n",p->totalCPUBursts);
				// printf("p completedCPUBursts: %d\n",p->completedCPUBursts);
				// printf("p completedIOBursts: %d\n",p->completedIOBursts);
				p->elapseTime(t);
				// printf("flag 2.5\n");

				p->priority = readyQCounter;
				readyQCounter++;
				readyQ.push(p);
				if (time < CUTOFF) printTime();
				if (time < CUTOFF) printf("Process %c completed I/O; added to ready queue ", idtoc(p->ID));
				if (time < CUTOFF) printReady();
				
			} else if (flag == 3) {
				// printf("flag 3\n");
				Process* p = incoming.top();
				int t = p->arrivalTime;
				elapseTime(t);
				incoming.pop();
				p->priority = readyQCounter;
				readyQCounter++;
				readyQ.push(p);
				if (time < CUTOFF) printTime();
				if (time < CUTOFF) printf("Process %c arrived; added to ready queue ", idtoc(p->ID));
				if (time < CUTOFF) printReady();
			}
			// printf("past flags\n");


			// printf("incoming queue: ");
			// printQueue(incoming);
			// printf("Ready Queue: ");
			// printQueue(readyQ);
			// if (inCPU != NULL) {
			// 	printf("CPU: %c\n",idtoc(inCPU->ID));
			// } else {
			// 	printf("CPU: --NULL--\n");
			// }
			// printf("IOBursts: ");
			// printQueue(IOBursts);

			// printf("\n\n\n\n");
		}
		// printf("out of loop\n");

		printTime();
		printf("Simulator ended for FCFS ");
		printReady();
	}

	void elapseTime(int t) {
		// printf("elapseTime: \n");
		time += t;
		if (inCPU != NULL) {
			inCPU->elapseTime(t);
		}

		if (CPUOpenIn != INT_MAX) {
			CPUOpenIn -= t;
			if (CPUOpenIn == 0) {
				CPUOpenIn = INT_MAX;
			}
		}

		vector<Process*> procs;
		while (!IOBursts.empty()) {
			Process* p = IOBursts.top();
			IOBursts.pop();
			p->elapseTime(t);
			procs.push_back(p);
		}
		for (size_t i = 0; i < procs.size(); i++) {
			IOBursts.push(procs[i]);
		}

		procs.clear();
		while (!incoming.empty()) {
			Process* p = incoming.top();
			incoming.pop();
			p->elapseTime(t);
			procs.push_back(p);
		}
		for (size_t i = 0; i < procs.size(); i++) {
			incoming.push(procs[i]);
		}




		
		// printf("end elapseTime: \n");
	}

	void printTime() {
		printf("time %ldms: ", time);
	}
	template<class S>
	void printQueue(priority_queue<Process*, vector<Process*>, S> queue) {
		priority_queue<Process*,vector<Process*>,S> copy = queue;
		
		while (!copy.empty()) {
			printf("%c ",idtoc(copy.top()->ID));
			copy.pop();
		}
		printf("\n");

	}

	void printReady() {
		priority_queue<Process*,vector<Process*>,Compare> copy = readyQ;
		if (copy.empty()) {
			printf("[Q <empty> ]\n");
		} else {
			printf("[Q ");
			while (!copy.empty()) {
				printf("%c ",idtoc(copy.top()->ID));
				copy.pop();
				
			}
			printf("]\n");
		}

	}
};

#endif