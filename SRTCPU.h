#ifndef SRTCPU_H
#define SRTCPU_H

#include "SRTprocess.h"
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

extern unsigned long CUTOFF;


class SRTCPU {
public:
	//INFO
	vector<SRTProcess*> processes;
	int ctxSwitchTime;


	//STATE
	unsigned long time=0;
	int readyQCounter = 0;


	//LOCATIONS
	priority_queue<SRTProcess*,vector<SRTProcess*>,SRTArrivalTimeCompare> incoming;
	priority_queue<SRTProcess*,vector<SRTProcess*>,SRTCompare> readyQ;
	priority_queue<SRTProcess*, vector<SRTProcess*>,SRTIOBurstTimeCompare> IOBursts;
	SRTProcess* cpu = NULL;
	SRTProcess* cpuOut = NULL;
	int ctxOutTime = INT_MAX;
	SRTProcess* cpuIn = NULL;
	int ctxInTime = INT_MAX;




	//METRICS
	int numIOCTXSwitches = 0;
	int numCPUCTXSwitches = 0;
	int numIOPreemptions = 0;
	int numCPUPreemptions = 0;

	int numCPUBoundProcesses = 0;
	int numIOBoundProcesses = 0;

	int cpuRunning = 0;



	SRTCPU(vector<SRTProcess*> procs, int switchTime) {
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
		if (cpu == NULL && cpuIn == NULL && cpuOut == NULL && !readyQ.empty()) {
			int flag = 5;
			return flag;
		}


		int CPU_TIME = INT_MAX;
		if (cpu != NULL)
			CPU_TIME = cpu->nextFinish();

		int CPU_OUT_TIME = INT_MAX;
		if (cpuOut != NULL)
			CPU_OUT_TIME = ctxOutTime;

		int CPU_IN_TIME = INT_MAX;
		if (cpuIn != NULL)
			CPU_IN_TIME = ctxInTime;
		


		int IO_FINISH = INT_MAX;
		if (!IOBursts.empty())
			IO_FINISH = IOBursts.top()->nextFinish();
		

		int INCOMING_FINISH = INT_MAX;
		if (!incoming.empty())
			INCOMING_FINISH = incoming.top()->arrivalTime;
		

		int min = INT_MAX;
		int flag = -1;


		if (CPU_TIME < min) {
			min = CPU_TIME;
			flag = 0;
			// printf("CPU_TIME: %d\n",CPU_TIME);
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
		printf("time %ldms: Simulator started for SRT ",time);
		printReady();
		while (!readyQ.empty() || !incoming.empty() || !IOBursts.empty() || cpu!=NULL || cpuIn!=NULL || cpuOut!=NULL ) {
			int flag = getNextEvent();
			// printf("flag: %d\n",flag);
			if (flag == 0) {
				// CPU FINISH
				// printf("1\n");
				// printReady();
				// printf("CPU: %c\nIOBursts: ",idtoc(cpu->ID));
				// printQueue(IOBursts);
				// printf("bool isfinished: %s\n", cpu->shouldTerminate() ? "true" : "false");
				int t = cpu->nextFinish();
				elapseTime(t,flag);
				// printf("2\n");

				//UPDATE METRICS
				if (cpu->isCPUBound)
					numCPUCTXSwitches++;
				else
					numIOCTXSwitches++;



				// printf("3\n");
				if (cpu->shouldTerminate()) {
					// printf("3.1\n");
					printTime();
					printf("Process %c terminated ", idtoc(cpu->ID));
					printReady();
					cpu->elapseTurnaroundTime(ctxSwitchTime/2);
					cpuOut = cpu;
					ctxOutTime = ctxSwitchTime/2;
				} else {
					// printf("3.2\n");
					cpuOut = cpu;
					ctxOutTime = ctxSwitchTime/2;
					if (time < CUTOFF) printTime();
					if (time < CUTOFF) printf("Process %c (tau %dms) completed a CPU burst; %d bursts to go ", idtoc(cpu->ID), cpu->getPriority(), cpu->totalCPUBursts - cpu->completedCPUBursts);
					if (time < CUTOFF) printReady();
					if (time < CUTOFF) printTime();
					int old_tau = cpuOut->getPriority();
					cpu->updatePriority();
					int new_tau = cpuOut->getPriority();

					if (time < CUTOFF) printf("Recalculating tau for process %c: old tau %dms ==> new tau %dms ", idtoc(cpu->ID), old_tau, new_tau);
					if (time < CUTOFF) printReady();
					if (time < CUTOFF) printTime();
					if (time < CUTOFF) printf("Process %c switching out of CPU; blocking on I/O until time %ldms ", idtoc(cpu->ID), cpu->nextFinish() + time + ctxSwitchTime/2);
					if (time < CUTOFF) printReady();
				}
				cpu = NULL;
			} 
			else if (flag == 1) {
				elapseTime(ctxOutTime,flag);

				// printf("ShoulDTerminate: Proc %c, %s\n",idtoc(cpuOut->ID),cpuOut->shouldTerminate()?"true": "false");
				// printf("cpuOut: %d %d\n",cpuOut->completedIOBursts,cpuOut->completedCPUBursts);
				// printf("CPU: %p\n",cpu);
				// printf("%p %p\n",&(cpuOut->bursts.front()), &(cpuOut->bursts.back()));
				if (cpuOut->nextFinish() != cpuOut->tempburst)
					readyQ.push(cpuOut);
				else if (!cpuOut->shouldTerminate())
					IOBursts.push(cpuOut);

				ctxOutTime = INT_MAX;
				cpuOut = NULL;
			} 
			else if (flag == 2) {
				elapseTime(ctxInTime,flag);

				cpu = cpuIn;
				cpuIn = NULL;
				ctxInTime = INT_MAX;

				// cpu->priority = 0;

				if (time < CUTOFF) printTime();
				if (cpu->nextFinish() == cpu->tempburst)  {
					if (time < CUTOFF) printf("Process %c (tau %dms) started using the CPU for %dms burst; ", idtoc(cpu->ID), cpu->getPriority(), cpu->nextFinish());
				} else {
					if (time < CUTOFF) printf("Process %c (tau %dms) started using the CPU for remaining %dms of %dms burst; ", idtoc(cpu->ID), cpu->getPriority(), cpu->nextFinish(), cpu->tempburst);
				}
				if (time < CUTOFF) printReady();
			} 
			else if (flag == 3) {
				SRTProcess* p = IOBursts.top();
				int t = p->nextFinish();
				IOBursts.pop();
				p->elapseTime(t);
				elapseTime(t,flag);

				// p->priority = readyQCounter;
				// readyQCounter++;
				readyQ.push(p);
				// printf("1\n");
				if (cpu != NULL) {
					// printf("1.1\n");
					if (p->priority < cpu->estTimeRemaining()) {
						// printf("1.11\n");
						if (cpu->isCPUBound) {
							numCPUCTXSwitches++;
							numCPUPreemptions++;
						} else {
							numIOCTXSwitches++;
							numIOPreemptions++;
						}
						// printf("1.112\n");
						cpuOut = cpu;
						// printf("1.113\n");
						cpu = NULL;
						ctxOutTime = ctxSwitchTime/2;
						// printf("1.114\n");
						if (time < CUTOFF) printTime();
						if (time < CUTOFF) printf("Process %c completed I/O; preempting %c ", idtoc(p->ID), idtoc(cpuOut->ID));
						if (time < CUTOFF) printReady();

					} else {
						// printf("1.12\n");
						if (time < CUTOFF) printTime();
						if (time < CUTOFF) printf("Process %c completed I/O; added to ready queue ", idtoc(p->ID));
						if (time < CUTOFF) printReady();
					}

				} else {
					// printf("1.2\n");
					// printf("should term?: %s\n",p->shouldTerminate()? "true": "false");

					if (time < CUTOFF) printTime();
					if (time < CUTOFF) printf("Process %c completed I/O; added to ready queue ", idtoc(p->ID));
					if (time < CUTOFF) printReady();
				}
			} 
			else if (flag == 4) {
				SRTProcess* p = incoming.top();
				incoming.pop();
				int t = p->arrivalTime;
				p->elapseTime(t);
				elapseTime(t,flag);

				readyQ.push(p);
				if (time < CUTOFF) printTime();
				if (time < CUTOFF) printf("Process %c (tau %dms) arrived; added to ready queue ", idtoc(p->ID), p->getPriority());
				if (time < CUTOFF) printReady();
			} 
			else if (flag == 5) {
				SRTProcess* p = readyQ.top();
				readyQ.pop();
				cpuIn = p;
				ctxInTime = ctxSwitchTime / 2;
			}
			// if (time < CUTOFF) printf("\n");
		}

		printTime();
		printf("Simulator ended for SRT ");
		printReady();

		/*
		Algorithm SRT
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

		int stdoutcpy = dup(1);
		close(1);
		int fd = open("simout.txt", O_APPEND | O_WRONLY | O_CREAT, 0660);
		if (fd == -1) {
			fprintf(stderr, "ERROR: could not open write file\n");
			exit(1);
		}

		printf("\nAlgorithm SRT\n");
		printf("-- CPU utilization: %.3f%%\n",ceil((100.0 * cpuRunning / time)*1000.0)/1000);
		printf("-- average CPU burst time: %.3f ms (%.3f ms/%.3f ms)\n",ceil((IOBOUND_cpu_burst_time + CPUBOUND_cpu_burst_time)/(double)(numIOBoundProcesses+numCPUBoundProcesses)*1000.0)/1000,ceil(CPUBOUND_cpu_burst_time/(double)numCPUBoundProcesses*1000.0)/1000,ceil(IOBOUND_cpu_burst_time/(double)numIOBoundProcesses*1000.0)/1000);
		printf("-- average wait time: %.3f ms (%.3f ms/%.3f ms)\n",ceil((CPU_wait + IO_wait)/(double)(numIOBoundProcesses+numCPUBoundProcesses)*1000.0)/1000,ceil(CPU_wait/(double)numCPUBoundProcesses*1000.0)/1000,ceil(IO_wait/(double)numIOBoundProcesses*1000.0)/1000);
		printf("-- average turnaround time: %.3f ms (%.3f ms/%.3f ms)\n",ceil((CPU_turnaround + IO_turnaround)/(double)(numIOBoundProcesses+numCPUBoundProcesses)*1000.0)/1000,ceil(CPU_turnaround/(double)numCPUBoundProcesses*1000.0)/1000,ceil(IO_turnaround/(double)numIOBoundProcesses*1000.0)/1000);
		printf("-- number of context switches: %d (%d/%d)\n",numIOCTXSwitches+numCPUCTXSwitches,numCPUCTXSwitches,numIOCTXSwitches);
		printf("-- number of preemptions: %d (%d/%d)\n",numIOPreemptions+numCPUPreemptions, numCPUPreemptions,numIOPreemptions);
		dup2(stdoutcpy, 1);
		close(stdoutcpy);

		

	}
	void elapseTime(int t, int flag) {
		time += t;
		elapseTimeCPU(t);
		elapseTimeIO(t);
		elapseTimeIncoming(t);
		elapseWaitTimeReady(t);
		if (cpu != NULL)
			cpuRunning += t;
		elapseTurnaroundTime(t);	
		
		// if (flag == 0) {
		// 	// CPU FINISH
		// } 
		// else if (flag == 1) {
		// 	// CPU OUT
		// } 
		// else if (flag == 2) {
		// 	// CPU IN
		// } 
		// else if (flag == 3) {
		// 	// IOBurst finish
		// } 
		// else if (flag == 4) {
		// 	// incoming finish
		// }
		// else if (flag == 5) {

		// }
	}



	void elapseTimeIO(int t) {
		vector<SRTProcess*> procs;
		while (!IOBursts.empty()) {
			SRTProcess* p = IOBursts.top();
			IOBursts.pop();
			p->elapseTime(t);
			procs.push_back(p);
		}
		for (size_t i = 0; i < procs.size(); i++) {
			IOBursts.push(procs[i]);
		}
	}
	void elapseTimeIncoming(int t) {
		vector<SRTProcess*> procs;
		while (!incoming.empty()) {
			SRTProcess* p = incoming.top();
			incoming.pop();
			p->elapseTime(t);
			procs.push_back(p);
		}
		for (size_t i = 0; i < procs.size(); i++) {
			incoming.push(procs[i]);
		}
	}
	void elapseTimeCPU(int t) {
		if (cpuOut != NULL) {
			ctxOutTime -= t;
		}

		if (cpu != NULL) {
			cpu->elapseTime(t);
			cpu->incTimeUsingCPU(t);
		}

		if (cpuIn != NULL)
			ctxInTime -= t;
	}
	void elapseWaitTimeReady(int t) {
		vector<SRTProcess*> procs;
		while (!readyQ.empty()) {
			SRTProcess* p = readyQ.top();
			readyQ.pop();
			p->elapseWaitTime(t);
			procs.push_back(p);
		}
		for (size_t i = 0; i < procs.size(); i++) {
			readyQ.push(procs[i]);
		}
	}
	void elapseTurnaroundTime(int t) {
		vector<SRTProcess*> procs;
		while (!readyQ.empty()) {
			SRTProcess* p = readyQ.top();
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
		priority_queue<SRTProcess*,vector<SRTProcess*>,SRTCompare> copy = readyQ;
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
	template<class S>
	void printQueue(priority_queue<SRTProcess*, vector<SRTProcess*>, S> queue) {
		priority_queue<SRTProcess*,vector<SRTProcess*>,S> copy = queue;
		
		while (!copy.empty()) {
			printf("%c ",idtoc(copy.top()->ID));
			copy.pop();
		}
		printf("\n");

	}
};

#endif