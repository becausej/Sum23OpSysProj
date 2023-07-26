#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <math.h>
#include <queue>
#include <stdlib.h>

#include "header.h"

using namespace std;


class FCFSCompare {
public:
    bool operator()(int* firstPair, int* secondPair)
    {
        return *firstPair < *secondPair;
    }
};

int main(int argc, char** argv){
    if(argc!=8){
        fprintf(stderr,"ERROR: Invalid arguments\n");
        return EXIT_FAILURE;
    }
    int processes = atoi(*(argv+1));
    int CPUProcesses = atoi(*(argv+2));
    int seed = atoi(*(argv+3));
    LAMBDA = atof(*(argv+4));
    UPPER_BOUND = atoi(*(argv+5));
    int ctxSwitchTime = atoi(*(argv+6));
    int timeSlice = atoi(*(argv+7));
    printf("%i %i\n",ctxSwitchTime,timeSlice);





    printf("<<< PROJECT PART I -- process set (n=%d) with %d CPU-bound process%s >>>\n",processes,CPUProcesses,CPUProcesses==1 ? "" : "es");
    srand48(seed);

    int* sizes = (int*) malloc(processes * sizeof(int));
    int** data = (int**) malloc(processes * sizeof(int*));


    // priority_queue<int*, int**, FCFSCompare> FCFS;
    //IO BOUND
    for(int i=0;i<processes-CPUProcesses;i++){
        int arrivalTime = next_exp(0);

        int num_bursts = (int)ceil(drand48()*64);
        *(sizes+i) = 2*num_bursts;
        *(data+i) = (int*) malloc((2*num_bursts) * sizeof(int));
        **(data+i) = arrivalTime;


        // printf("I/O-bound process %c: arrival time %dms; %d CPU bursts:\n",(char)(i+65),arrivalTime,*(data+i));
        for(int j =0;j<num_bursts;j++){
            int CPUBurst = next_exp(1);
            *(*(data+i)+2*j + 1) = CPUBurst;
            if(j!=num_bursts-1){
                int IOBurst = next_exp(1)*10;
                *(*(data+i)+2*j+1) = IOBurst;
                // printf("--> CPU burst %dms --> I/O burst %dms\n",CPUBurst,IOBurst);
            }
            else{
                // printf("--> CPU burst %dms\n",CPUBurst);
            }

        }
    }
    //CPU BOUND
    for(int i=processes-CPUProcesses;i<processes;i++){
        int arrivalTime = next_exp(0);
        int num_bursts = (int)ceil(drand48()*64);
        *(sizes+i) = 2*num_bursts;
        *(data+i) = (int*) malloc((2*num_bursts) * sizeof(int));
        **(data+i) = arrivalTime;


        // printf("CPU-bound process %c: arrival time %dms; %d CPU bursts:\n",(char)(i+processes-CPUProcesses+65),arrivalTime,*(data+i));
        for(int j =0;j<num_bursts;j++){
            int CPUBurst = next_exp(1)*4;
            *(*(data+i)+2*j + 1) = CPUBurst;

            if(j!=num_bursts-1){
                int IOBurst = next_exp(1)*10/8;
                *(*(data+i)+2*j + 2) = IOBurst;
                // printf("--> CPU burst %dms --> I/O burst %dms\n",CPUBurst,IOBurst);
            }
            else{
                // printf("--> CPU burst %dms\n",CPUBurst);
            }

        }
    }
}