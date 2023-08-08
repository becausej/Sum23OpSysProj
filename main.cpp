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

double LAMBDA;
int UPPER_BOUND;
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

#include "process.h"
#include "FCFSCPU.h"

void FCFS(vector<Process*> processes, int ctxSwitchTime) {
    FCFSCPU cpu(processes, ctxSwitchTime);
    cpu.run();
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
    double alpha = atof(*(argv+7));
    int timeSlice = atoi(*(argv+8));
    printf("alpha: %f\n",alpha);

    printf("<<< PROJECT PART I -- process set (n=%d) with %d CPU-bound process%s >>>\n",numProcesses,numCPUProcesses,numCPUProcesses==1 ? "" : "es");
    srand48(seed);
    vector<Process*> processes;


    //IO BOUND
    for(int i=0;i<numProcesses-numCPUProcesses;i++){
        int arrivalTime = next_exp(0);
        int num_bursts = (int)ceil(drand48()*64);

        processes.push_back(new Process(i,arrivalTime,num_bursts,false));

        printf("I/O-bound process %c: arrival time %dms; %d CPU bursts:\n",(char)(i+65),arrivalTime,num_bursts);
        
    }
    //CPU BOUND
    for(int i=numProcesses-numCPUProcesses;i<numProcesses;i++){
        int arrivalTime = next_exp(0);
        int num_bursts = (int)ceil(drand48()*64);
        
        processes.push_back(new Process(i,arrivalTime,num_bursts,true));



        printf("CPU-bound process %c: arrival time %dms; %d CPU bursts\n",(char)(i+65),arrivalTime,num_bursts);
        
    }
    printf("\n");
    printf("<<< PROJECT PART II -- t_cs=%dms; alpha=%f; t_slice=%dms >>>\n",ctxSwitchTime, alpha, timeSlice);
    FCFS(processes, ctxSwitchTime);

    // for (size_t i = 0; i < processes.size(); i++) {
    //     free(processes[i]);
    // }

}