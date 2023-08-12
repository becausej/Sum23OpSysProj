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
#include <climits>


using namespace std;

char idtoc(int id ) {
    return (char) (id + 65);
}


unsigned long CUTOFF = 10000;
float LAMBDA;
int UPPER_BOUND;
int TIME_SLICE;
float ALPHA;

float next_exp(int mode){
    int cont = 1;
    float toReturn;
    while(cont){
        float result = drand48();
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



int main(int argc, char** argv){
    // close(1);
    // open("out.txt",O_WRONLY | O_CREAT | O_TRUNC, 0660);
    // freopen("out.txt","w",stdout);
    if(argc!=9) {
        fprintf(stdout,"ERROR: Invalid arguments\n"); return EXIT_FAILURE;
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
    if (numProcesses <= 0 || numCPUProcesses > numProcesses || numCPUProcesses < 0 || UPPER_BOUND <= 0 || TIME_SLICE <= 0 || ALPHA > 1 || ALPHA < 0) {
        fprintf(stdout,"ERROR: Invalid arguments\n"); return EXIT_FAILURE;
    }

    printf("<<< PROJECT PART I -- process set (n=%d) with %d CPU-bound process%s >>>\n",numProcesses,numCPUProcesses,numCPUProcesses==1 ? "" : "es");


    vector<FCFSProcess*> FCFSprocesses;

    srand48(seed);
    //IO BOUND
    for(int i=0;i<numProcesses-numCPUProcesses;i++){
        int arrivalTime = next_exp(0);
        int num_bursts = (int)ceil(drand48()*64);
        FCFSprocesses.push_back(new FCFSProcess(i,arrivalTime,num_bursts,false));
        printf("I/O-bound process %c: arrival time %dms; %d CPU bursts\n",(char)(i+65),arrivalTime,num_bursts);
    }
    //CPU BOUND
    for(int i=numProcesses-numCPUProcesses;i<numProcesses;i++){
        int arrivalTime = next_exp(0);
        int num_bursts = (int)ceil(drand48()*64);
        FCFSprocesses.push_back(new FCFSProcess(i,arrivalTime,num_bursts,true));
        printf("CPU-bound process %c: arrival time %dms; %d CPU bursts\n",(char)(i+65),arrivalTime,num_bursts);
    }
    printf("\n");
    printf("<<< PROJECT PART II -- t_cs=%dms; alpha=%0.2f; t_slice=%dms >>>\n",ctxSwitchTime, ALPHA, TIME_SLICE);
    FCFS(FCFSprocesses, ctxSwitchTime);
    for (int i = 0; i < numProcesses; i++) {
        delete FCFSprocesses[i];
    }
    FCFSprocesses.clear();




    vector<SJFProcess*> SJFprocesses;
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
    printf("\n");
    SJF(SJFprocesses, ctxSwitchTime);
    for (int i = 0; i < numProcesses; i++) {
        delete SJFprocesses[i];
    }
    SJFprocesses.clear();




    vector<SRTProcess*> SRTprocesses;
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
    printf("\n");
    SRT(SRTprocesses, ctxSwitchTime);
    for (int i = 0; i < numProcesses; i++) {
        delete SRTprocesses[i];
    }
    SRTprocesses.clear();





    vector<RRProcess*> RRprocesses;
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
    printf("\n");
    RR(RRprocesses, ctxSwitchTime);

    for (int i = 0; i < numProcesses; i++) {
        delete RRprocesses[i];
    }
    RRprocesses.clear();
    // close(1);
}