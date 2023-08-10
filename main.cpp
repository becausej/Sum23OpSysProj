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


using namespace std;

char idtoc(int id ) {
    return (char) (id + 65);
}


unsigned long CUTOFF = 10000;
double LAMBDA;
int UPPER_BOUND;
int TIME_SLICE;
double ALPHA;

double next_exp(int mode){
    int cont = 1;
    double toReturn;
    while(cont){
        double result = drand48();
        toReturn = -log(result)/LAMBDA;
        if(mode==0){
            toReturn = floor(toReturn);
            if(toReturn <= UPPER_BOUND){
                cont = 0;
            }
        }
        else{
            toReturn = ceil(toReturn);
            if(toReturn <= UPPER_BOUND){
                cont = 0;
            }
        }
    }
    return toReturn;}


#include "FCFSCPU.h"
#include "SJFCPU.h"
#include "SRTCPU.h"
#include "RRCPU.h"

#include "FCFSprocess.h"
#include "SJFprocess.h"
#include "SRTprocess.h"
#include "RRprocess.h"







void FCFS(vector<FCFSProcess*> processes, int ctxSwitchTime) {
    FCFSCPU cpu(processes, ctxSwitchTime);
    cpu.run();
}
void SJF(vector<SJFProcess*> processes, int ctxSwitchTime) {
    SJFCPU cpu(processes, ctxSwitchTime);
    cpu.run();
}
void SRT(vector<SRTProcess*> processes, int ctxSwitchTime) {
    SRTCPU cpu(processes, ctxSwitchTime);
    cpu.run();
}
void RR(vector<RRProcess*> processes, int ctxSwitchTime) {
    RRCPU cpu(processes, ctxSwitchTime);
    cpu.run();
}

void genProcesses(
    int seed,
    int numProcesses, 
    int numCPUProcesses, 
    vector<FCFSProcess*>& FCFSprocesses,
    vector<SJFProcess*>& SJFprocesses,
    vector<SRTProcess*>& SRTprocesses,
    vector<RRProcess*>& RRprocesses) {




    srand48(seed);
    //IO BOUND
    for(int i=0;i<numProcesses-numCPUProcesses;i++){
        int arrivalTime = next_exp(0);
        int num_bursts = (int)ceil(drand48()*64);
        FCFSprocesses.push_back(new FCFSProcess(i,arrivalTime,num_bursts,false));
        printf("I/O-bound process %c: arrival time %dms; %d CPU bursts:\n",(char)(i+65),arrivalTime,num_bursts);
    }
    //CPU BOUND
    for(int i=numProcesses-numCPUProcesses;i<numProcesses;i++){
        int arrivalTime = next_exp(0);
        int num_bursts = (int)ceil(drand48()*64);
        FCFSprocesses.push_back(new FCFSProcess(i,arrivalTime,num_bursts,true));
        printf("CPU-bound process %c: arrival time %dms; %d CPU bursts\n",(char)(i+65),arrivalTime,num_bursts);
    }




    srand48(seed);
    //IO BOUND
    for(int i=0;i<numProcesses-numCPUProcesses;i++){
        int arrivalTime = next_exp(0);
        int num_bursts = (int)ceil(drand48()*64);
        SJFprocesses.push_back(new SJFProcess(i,arrivalTime,num_bursts,false));
        // printf("I/O-bound process %c: arrival time %dms; %d CPU bursts:\n",(char)(i+65),arrivalTime,num_bursts);
    }
    //CPU BOUND
    for(int i=numProcesses-numCPUProcesses;i<numProcesses;i++){
        int arrivalTime = next_exp(0);
        int num_bursts = (int)ceil(drand48()*64);
        SJFprocesses.push_back(new SJFProcess(i,arrivalTime,num_bursts,true));
        // printf("CPU-bound process %c: arrival time %dms; %d CPU bursts\n",(char)(i+65),arrivalTime,num_bursts);
    }




    srand48(seed);
    //IO BOUND
    for(int i=0;i<numProcesses-numCPUProcesses;i++){
        int arrivalTime = next_exp(0);
        int num_bursts = (int)ceil(drand48()*64);
        SRTprocesses.push_back(new SRTProcess(i,arrivalTime,num_bursts,false));
        // printf("I/O-bound process %c: arrival time %dms; %d CPU bursts:\n",(char)(i+65),arrivalTime,num_bursts);
    }
    //CPU BOUND
    for(int i=numProcesses-numCPUProcesses;i<numProcesses;i++){
        int arrivalTime = next_exp(0);
        int num_bursts = (int)ceil(drand48()*64);
        SRTprocesses.push_back(new SRTProcess(i,arrivalTime,num_bursts,true));
        // printf("CPU-bound process %c: arrival time %dms; %d CPU bursts\n",(char)(i+65),arrivalTime,num_bursts);
    }




    srand48(seed);
    //IO BOUND
    for(int i=0;i<numProcesses-numCPUProcesses;i++){
        int arrivalTime = next_exp(0);
        int num_bursts = (int)ceil(drand48()*64);
        RRprocesses.push_back(new RRProcess(i,arrivalTime,num_bursts,false));
        // printf("I/O-bound process %c: arrival time %dms; %d CPU bursts:\n",(char)(i+65),arrivalTime,num_bursts);
    }
    //CPU BOUND
    for(int i=numProcesses-numCPUProcesses;i<numProcesses;i++){
        int arrivalTime = next_exp(0);
        int num_bursts = (int)ceil(drand48()*64);
        RRprocesses.push_back(new RRProcess(i,arrivalTime,num_bursts,true));
        // printf("CPU-bound process %c: arrival time %dms; %d CPU bursts\n",(char)(i+65),arrivalTime,num_bursts);
    }
}



int main(int argc, char** argv){
    if(argc!=9) {
        fprintf(stderr,"ERROR: Invalid arguments\n"); return EXIT_FAILURE;
    }
    int numProcesses = atoi(*(argv+1));
    int numCPUProcesses = atoi(*(argv+2));
    int seed = atoi(*(argv+3));
    LAMBDA = atof(*(argv+4));
    UPPER_BOUND = atoi(*(argv+5));
    int ctxSwitchTime = atoi(*(argv+6));
    ALPHA = atof(*(argv+7));
    TIME_SLICE = atoi(*(argv+8));
    // printf("alpha: %f\n",alpha);

    printf("<<< PROJECT PART I -- process set (n=%d) with %d CPU-bound process%s >>>\n",numProcesses,numCPUProcesses,numCPUProcesses==1 ? "" : "es");


    vector<FCFSProcess*> FCFSprocesses;
    vector<SJFProcess*> SJFprocesses;
    vector<SRTProcess*> SRTprocesses;
    vector<RRProcess*> RRprocesses;
    genProcesses(seed, numProcesses, numCPUProcesses, FCFSprocesses, SJFprocesses, SRTprocesses, RRprocesses);


    printf("\n");
    printf("<<< PROJECT PART II -- t_cs=%dms; alpha=%f; t_slice=%dms >>>\n",ctxSwitchTime, ALPHA, TIME_SLICE);
    FCFS(FCFSprocesses, ctxSwitchTime);
    printf("\n");
    SJF(SJFprocesses, ctxSwitchTime);
    printf("\n");
    SRT(SRTprocesses, ctxSwitchTime);
    printf("\n");
    RR(RRprocesses, ctxSwitchTime);

    for (int i = 0; i < numProcesses; i++) {
        delete FCFSprocesses[i];
        delete SJFprocesses[i];
        delete SRTprocesses[i];
        delete RRprocesses[i];
    }
}