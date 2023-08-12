#ifndef RRCPU_H
#define RRCPU_H

#include "RRprocess.h"
#include <math.h>
#include <numeric>
#include <climits>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>

extern unsigned long CUTOFF;
extern int TIME_SLICE;


class RRCPU {
public:
	//INFO
	vector<RRProcess*> processes;
	int ctxSwitchTime;

	//STATE
	unsigned long time=0;
	int readyQCounter = 0;

	//LOCATIONS
	priority_queue<RRProcess*,vector<RRProcess*>,RRArrivalTimeCompare> incoming;
	priority_queue<RRProcess*,vector<RRProcess*>,RRCompare> readyQ;
	priority_queue<RRProcess*, vector<RRProcess*>,RRIOBurstTimeCompare> IOBursts;
	RRProcess* cpu = NULL;
	RRProcess* cpuOut = NULL;
	int ctxOutTime = INT_MAX;
	RRProcess* cpuIn = NULL;
	int ctxInTime = INT_MAX;

	//METRICS
	int numIOCTXSwitches = 0;
	int numCPUCTXSwitches = 0;
	int numIOPreemptions = 0;
	int numCPUPreemptions = 0;

	int numCPUBoundProcesses = 0;
	int numIOBoundProcesses = 0;

	int cpuRunning = 0;

	int timeslice = TIME_SLICE;



	RRCPU(vector<RRProcess*> procs, int switchTime) {
		ctxSwitchTime = switchTime;
		for (size_t i = 0; i < procs.size(); i++) {
			incoming.push(procs[i]);
			if (procs[i]->isCPUBound)
				numCPUBoundProcesses+=procs[i]->totalCPUBursts;
			else
				numIOBoundProcesses+=procs[i]->totalCPUBursts;
		}

		processes = procs;
	}

	int getNextEvent() {
		if (cpu == NULL && ctxInTime == INT_MAX && ctxOutTime == INT_MAX && !readyQ.empty()) {
			int flag = 5;
			return flag;
		}


		int CPU_TIME = INT_MAX;
		if (cpu != NULL && ctxOutTime == INT_MAX)
			CPU_TIME = cpu->nextFinish();

		int CPU_OUT_TIME = INT_MAX;
		if (ctxOutTime != INT_MAX)
			CPU_OUT_TIME = ctxOutTime;

		int CPU_IN_TIME = INT_MAX;
		if (ctxInTime != INT_MAX)
			CPU_IN_TIME = ctxInTime;
		


		int IO_FINISH = INT_MAX;
		if (!IOBursts.empty())
			IO_FINISH = IOBursts.top()->nextFinish();
		

		int INCOMING_FINISH = INT_MAX;
		if (!incoming.empty())
			INCOMING_FINISH = incoming.top()->arrivalTime;
		
		int CPU_HALF_IN = INT_MAX;
		if (ctxInTime != INT_MAX && cpuIn == NULL)
			CPU_HALF_IN = ctxInTime - ceil((ctxSwitchTime / 2) / 2.0);

		int CPU_HALF_OUT = INT_MAX;
		if (ctxOutTime != INT_MAX && cpuOut == NULL)
			CPU_HALF_OUT = ctxOutTime - ceil((ctxSwitchTime / 2) / 2.0);
		

		int min = INT_MAX;
		int flag = -1;

		if (CPU_HALF_OUT < min) {
			min = CPU_HALF_OUT;
			flag = -3;
		}
		if (CPU_HALF_IN < min) {
			min = CPU_HALF_IN;
			flag = -2;
		}


		if (CPU_TIME < min) {
			min = CPU_TIME;
			flag = 0;
		} 
		if (cpu != NULL && ctxOutTime == INT_MAX && timeslice < min) {
			min = timeslice;
			flag = -1;
		}


		if (CPU_OUT_TIME < min) {
			min = CPU_OUT_TIME;
			flag = 1;
		} 
		if (CPU_IN_TIME < min) {
			min = CPU_IN_TIME;
			flag = 2;
		} 
		if (IO_FINISH < min) {
			min = IO_FINISH;
			flag = 3;
		} 
		if (INCOMING_FINISH < min) {
			min = INCOMING_FINISH;
			flag = 4;
		}

		return flag;

	}


	void run() {
		printf("time %ldms: Simulator started for RR ",time);
		printReady();
		while (!readyQ.empty() || !incoming.empty() || !IOBursts.empty() || cpu!=NULL || cpuOut != NULL || cpuIn != NULL ) {
			int flag = getNextEvent();

			// if (time < CUTOFF) printf("flag: %d\n",flag);
			// if (time < CUTOFF) printf("timeslice: %d\n",timeslice);
			if (flag == -3) {
				elapseTime(ctxOutTime - ceil((ctxSwitchTime / 2) / 2.0),flag);
				cpuOut = cpu;
				cpu = NULL;
				// if (cpuOut->isCPUBound)
				// 	numCPUCTXSwitches++;
				// else
				// 	numIOCTXSwitches++;
				
			} else if (flag == -2) {
				elapseTime(ctxInTime - ceil((ctxSwitchTime / 2) / 2.0),flag);
				cpuIn = readyQ.top();
				readyQ.pop();
			} else if (flag == -1) {
				if (!readyQ.empty() || (ctxInTime!=INT_MAX)) {
					int t = timeslice;
					elapseTime(t,flag);

					if (time < CUTOFF) printTime();
					if (time < CUTOFF) printf("Time slice expired; preempting process %c with %dms remaining ", idtoc(cpu->ID), cpu->nextFinish());
					if (time < CUTOFF) printReady();


					

					if (cpu->isCPUBound) {
						numCPUPreemptions++;
						numCPUCTXSwitches++;
					} else {
						numIOPreemptions++;
						numIOCTXSwitches++;
					}
					timeslice = TIME_SLICE;
					// cpuOut = cpu;
					ctxOutTime = ctxSwitchTime/2;
					// cpu = NULL;
				} else {
					int t = timeslice;
					elapseTime(t,flag);
					if (time < CUTOFF) printTime();
					if (time < CUTOFF) printf("Time slice expired; no preemption because ready queue is empty ");
					if (time < CUTOFF) printReady();
					timeslice = TIME_SLICE;
				}
			}
			else if (flag == 0) {
				// CPU FINISH
				elapseTime(cpu->nextFinish(),flag);

				//UPDATE METRICS
				// if (cpu->isCPUBound)
				// 	numCPUCTXSwitches++;
				// else
				// 	numIOCTXSwitches++;



				ctxOutTime = ctxSwitchTime/2;
				if (cpu->shouldTerminate()) {
					printTime();
					printf("Process %c terminated ", idtoc(cpu->ID));
					printReady();
					// printQueue(readyQ);
					// cpu->elapseTurnaroundTime(ctxSwitchTime/2);
					// cpuOut = cpu;
				} else {
					// cpuOut = cpu;
					if (time < CUTOFF) printTime();
					if (time < CUTOFF) printf("Process %c completed a CPU burst; %d burst%s to go ", idtoc(cpu->ID), cpu->totalCPUBursts - cpu->completedCPUBursts, (cpu->totalCPUBursts - cpu->completedCPUBursts)==1 ? "" : "s");
					if (time < CUTOFF) printReady();
					if (time < CUTOFF) printTime();
					if (time < CUTOFF) printf("Process %c switching out of CPU; blocking on I/O until time %ldms ", idtoc(cpu->ID), cpu->nextFinish() + time + ctxSwitchTime/2);
					if (time < CUTOFF) printReady();
				}
				// cpu = NULL;
				timeslice = TIME_SLICE;
			} 
			else if (flag == 1) {
				elapseTime(ctxOutTime,flag);
				

				if (!cpuOut->shouldTerminate()) {
					if (cpuOut->completedCPUBursts == cpuOut->completedIOBursts) {
						cpuOut->priority = readyQCounter;
						readyQCounter++;
						readyQ.push(cpuOut);
						cpuOut->got_preempted = false;
					} else {
						IOBursts.push(cpuOut);
					}
				}

				ctxOutTime = INT_MAX;
				cpuOut = NULL;
			} 
			else if (flag == 2) {
				elapseTime(ctxInTime,flag);

				cpu = cpuIn;
				cpuIn = NULL;
				ctxInTime = INT_MAX;


				cpu->priority = 0;

				if (time < CUTOFF) printTime();
				// if (cpu->got_preempted)  {
				if (cpu->nextFinish() == cpu->tempburst && !cpu->got_preempted) {
					if (time < CUTOFF) printf("Process %c started using the CPU for %dms burst ", idtoc(cpu->ID), cpu->nextFinish());
				} else {
					if (time < CUTOFF) printf("Process %c started using the CPU for remaining %dms of %dms burst ", idtoc(cpu->ID), cpu->nextFinish(), cpu->tempburst);
				}
				if (time < CUTOFF) printReady();
				// if (time < CUTOFF) printQueue(readyQ);
				// if (time < CUTOFF) printQueue(IOBursts);
			} 
			else if (flag == 3) {
				RRProcess* p = IOBursts.top();
				int t = p->nextFinish();
				IOBursts.pop();
				p->elapseTime(t,flag);
				elapseTime(t,flag);

				p->priority = readyQCounter;
				readyQCounter++;
				readyQ.push(p);
				if (time < CUTOFF) printTime();
				if (time < CUTOFF) printf("Process %c completed I/O; added to ready queue ", idtoc(p->ID));
				if (time < CUTOFF) printReady();
			} 
			else if (flag == 4) {
				RRProcess* p = incoming.top();
				incoming.pop();
				int t = p->arrivalTime;
				p->elapseTime(t,flag);
				elapseTime(t,flag);
				p->priority = readyQCounter;
				readyQCounter++;
				readyQ.push(p);
				if (time < CUTOFF) printTime();
				if (time < CUTOFF) printf("Process %c arrived; added to ready queue ", idtoc(p->ID));
				if (time < CUTOFF) printReady();
			} 
			else if (flag == 5) {
				// RRProcess* p = readyQ.top();
				// readyQ.pop();
				// cpuIn = p;
				ctxInTime = ctxSwitchTime / 2;
			}
			// if (time < CUTOFF) printQueue(readyQ);
			// if (time < CUTOFF) printQueue(IOBursts);
			// if (time < CUTOFF) printf("cpu: %c\n",cpu == NULL ? '-' : idtoc(cpu->ID));
			// if (time < CUTOFF) printf("\n");
		}

		printTime();
		printf("Simulator ended for RR ");
		printReady();

		/*
		Algorithm RR
		-- CPU utilization: 84.253%
		-- average CPU burst time: 3067.776 ms (4071.000 ms/992.138 ms)
		-- average wait time: 779.663 ms (217.284 ms/1943.207 ms)
		-- average turnaround time: 3851.439 ms (4292.284 ms/2939.345 ms)
		-- number of context switches: 89 (60/29)
		-- number of preemptions: 0 (0/0)
		*/

		long CPUBOUND_cpu_burst_time = 0;
		long IOBOUND_cpu_burst_time = 0;
		for (size_t i = 0; i < processes.size(); i++) {
			if (processes[i]->isCPUBound)
				CPUBOUND_cpu_burst_time += processes[i]->time_using_cpu;
			else
				IOBOUND_cpu_burst_time += processes[i]->time_using_cpu;
		}

		long CPU_turnaround = 0;
		long IO_turnaround = 0;
		for (size_t i = 0; i < processes.size(); i++) {
			if (processes[i]->isCPUBound)
				CPU_turnaround += processes[i]->total_turnaround_time;
			else
				IO_turnaround += processes[i]->total_turnaround_time;
		}

		long CPU_wait = 0;
		long IO_wait = 0;
		for (size_t i = 0; i < processes.size(); i++) {
			if (processes[i]->isCPUBound)
				CPU_wait += processes[i]->total_wait_time;
			else
				IO_wait += processes[i]->total_wait_time;
		}
		double cu = time? (ceil((100.0 * cpuRunning / time)*1000.0))/1000.0 : 0.0;
		double cbt1 = (numIOBoundProcesses+numCPUBoundProcesses) ? (ceil(((IOBOUND_cpu_burst_time + CPUBOUND_cpu_burst_time)/(double)(numIOBoundProcesses+numCPUBoundProcesses))*1000.0))/1000.0 : 0.0;
		double cbt2 = numCPUBoundProcesses ? (ceil(CPUBOUND_cpu_burst_time/(double)numCPUBoundProcesses*1000.0))/1000.0 : 0.0;
		double cbt3 = numIOBoundProcesses ? (ceil(IOBOUND_cpu_burst_time/(double)numIOBoundProcesses*1000.0))/1000.0 : 0.0;
		double awt1 = (numIOBoundProcesses+numCPUBoundProcesses) ? ( ceil((CPU_wait + IO_wait)/(double)(numIOBoundProcesses+numCPUBoundProcesses)*1000.0))/1000.0 : 0.0;
		double awt2 = numCPUBoundProcesses ? ( ceil(CPU_wait/(double)numCPUBoundProcesses*1000.0))/1000.0 : 0.0;
		double awt3 = numIOBoundProcesses ? ( ceil(IO_wait/(double)numIOBoundProcesses*1000.0))/1000.0 : 0.0;
		double att1 = (numIOBoundProcesses+numCPUBoundProcesses) ? ( ceil((CPU_turnaround + IO_turnaround)/(double)(numIOBoundProcesses+numCPUBoundProcesses)*1000.0))/1000.0 : 0.0;
		double att2 = numCPUBoundProcesses ? ( ceil(CPU_turnaround/(double)numCPUBoundProcesses*1000.0))/1000.0 : 0.0;
		double att3 = numIOBoundProcesses ? ( ceil(IO_turnaround/(double)numIOBoundProcesses*1000.0))/1000.0 : 0.0;
		ofstream output;
		output.open("simout.txt", ios::out | ios::app);
		output.setf(ios::fixed,ios::floatfield);
		output.precision(3);
		output << "Algorithm RR\n";
		output << "-- CPU utilization: " << cu << "%\n";
		output << "-- average CPU burst time: " << cbt1 << " ms (" << cbt2 << " ms/" << cbt3 << " ms)\n";
		output << "-- average wait time: " << awt1 << " ms (" << awt2 << " ms/" << awt3 << " ms)\n";
		output << "-- average turnaround time: " << att1 << " ms (" << att2 << " ms/" << att3 << " ms)\n";
		output << "-- number of context switches: " << numIOCTXSwitches+numCPUCTXSwitches << " (" << numCPUCTXSwitches << "/" << numIOCTXSwitches << ")\n";
		output << "-- number of preemptions: " << numIOPreemptions+numCPUPreemptions << " (" << numCPUPreemptions << "/" << numIOPreemptions << ")";
		output.close();

		

	}
	void elapseTime(int t, int flag) {
		time += t;
		elapseTimeCPU(t,flag);
		elapseTimeIO(t,flag);
		elapseTimeIncoming(t,flag);
		elapseWaitTimeReady(t);
		if (cpu != NULL && ctxOutTime == INT_MAX) {
			cpuRunning += t;
			timeslice -= t;
		}
		elapseTurnaroundTime(t);
		if (flag == -1){
			timeslice = TIME_SLICE;
		}
	}
	void elapseTimeIO(int t,int flag) {
		vector<RRProcess*> procs;
		int ID = -1;
		if (!readyQ.empty())
			ID = readyQ.top()->ID;
		while (!IOBursts.empty()) {
			RRProcess* p = IOBursts.top();
			IOBursts.pop();
			p->elapseTime(t,ID == p->ID ? flag : -1);
			procs.push_back(p);
		}
		for (size_t i = 0; i < procs.size(); i++) {
			IOBursts.push(procs[i]);
		}
	}
	void elapseTimeIncoming(int t,int flag) {
		vector<RRProcess*> procs;
		while (!incoming.empty()) {
			RRProcess* p = incoming.top();
			incoming.pop();
			p->elapseTime(t,flag);
			procs.push_back(p);
		}
		for (size_t i = 0; i < procs.size(); i++) {
			incoming.push(procs[i]);
		}
	}
	void elapseTimeCPU(int t,int flag) {
		if (ctxOutTime != INT_MAX)
			ctxOutTime -= t;

		if (cpu != NULL && ctxOutTime == INT_MAX) {
			cpu->elapseTime(t,flag);
			cpu->time_using_cpu += t;
		}

		if (ctxInTime != INT_MAX)
			ctxInTime -= t;
	}
	void elapseWaitTimeReady(int t) {
		vector<RRProcess*> procs;
		int ID = -1;
		if (!readyQ.empty() && ctxInTime!= INT_MAX && cpuIn == NULL)
			ID = readyQ.top()->ID;
		while (!readyQ.empty()) {
			RRProcess* p = readyQ.top();
			readyQ.pop();
			if (p->ID != ID)
				p->elapseWaitTime(t);
			procs.push_back(p);
		}
		for (size_t i = 0; i < procs.size(); i++) {
			readyQ.push(procs[i]);
		}
	}
	void elapseTurnaroundTime(int t) {
		vector<RRProcess*> procs;
		while (!readyQ.empty()) {
			RRProcess* p = readyQ.top();
			readyQ.pop();
			p->elapseTurnaroundTime(t);
			procs.push_back(p);
		}
		for (size_t i = 0; i < procs.size(); i++) {
			readyQ.push(procs[i]);
		}


		if (cpuOut != NULL)
			cpuOut->elapseTurnaroundTime(t);

		if (cpu != NULL) {
			cpu->elapseTurnaroundTime(t);
		}

		if (cpuIn != NULL)
			cpuIn->elapseTurnaroundTime(t);
	}


	void printTime() {
		printf("time %ldms: ", time);
	}

	void printReady() {
		priority_queue<RRProcess*,vector<RRProcess*>,RRCompare> copy = readyQ;
		if (copy.empty()) {
			printf("[Q <empty>]\n");
		} else {
			printf("[Q ");
			while (!copy.empty()) {
				if (copy.size() != 1)
					printf("%c ",idtoc(copy.top()->ID));
				else
					printf("%c",idtoc(copy.top()->ID));
				copy.pop();
				
			}
			printf("]\n");
		}

	}
	template<class S>
	void printQueue(priority_queue<RRProcess*, vector<RRProcess*>, S> queue) {
		priority_queue<RRProcess*,vector<RRProcess*>,S> copy = queue;
		
		while (!copy.empty()) {
			printf("%c ",idtoc(copy.top()->ID));
			copy.pop();
		}
		copy= queue;
		printf("\n");
		while (!copy.empty()) {
			printf("%d ",copy.top()->priority);
			copy.pop();
		}
		copy= queue;
		printf("\n");
		while (!copy.empty()) {
			printf("%d ",copy.top()->nextFinish());
			copy.pop();
		}
		printf("\n");

	}
};

#endif