#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <math.h>
#include <queue>
using namespace std;


class Compare {
public:
    bool operator()(pair<int,int> firstPair, pair<int,int> secondPair)
    {
        if (firstPair.second < secondPair.second) {
            return true;
        }
        return false;
    }
};



double lambda;
int upperBound;
//0 is floor 1 is ceil
double next_exp(int mode){
    int cont = 1;
    double toReturn;
    while(cont){
        double result = drand48();
        toReturn = -log(result)/lambda;
        if(mode==0){
            toReturn = floor(toReturn);
            if(toReturn <= upperBound){
                cont = 0;
            }
        }
        else{
            toReturn = ceil(toReturn);
            if(toReturn <= upperBound){
                cont = 0;
            }
        }
    }
    return toReturn;
}

int main(int argc, char** argv){
    if(argc!=6){
        fprintf(stderr,"ERROR: Invalid arguments\n");
        return EXIT_FAILURE;
    }
    argv++;
    int processes = atoi(*argv);
    argv++;
    int CPUProcesses = atoi(*argv);
    argv++;
    int seed = atoi(*argv);
    argv++;
    lambda = atof(*argv);
    argv++;
    upperBound = atoi(*argv);

    priority_queue<pair<int,int>, vector<pair<int,int>>, Compare> queue;
    if(CPUProcesses==1){
        printf("<<< PROJECT PART I -- process set (n=%d) with %d CPU-bound process >>>\n",processes,CPUProcesses);
    }
    else{
        printf("<<< PROJECT PART I -- process set (n=%d) with %d CPU-bound processes >>>\n",processes,CPUProcesses);
    }
    srand48(seed);
    //IO BOUND
    for(int i=0;i<processes-CPUProcesses;i++){
        int arrivalTime = next_exp(0);
        int bursts = (int)ceil(drand48()*64);
        printf("I/O-bound process %c: arrival time %dms; %d CPU bursts:\n",(char)(i+65),arrivalTime,bursts);
        for(int j =0;j<bursts;j++){
            int CPUBurst = next_exp(1);
            if(j!=bursts-1){
                int IOBurst = next_exp(1)*10;
                printf("--> CPU burst %dms --> I/O burst %dms\n",CPUBurst,IOBurst);
            }
            else{
                printf("--> CPU burst %dms\n",CPUBurst);
            }

        }
    }
    //CPU BOUND
    for(int i=0;i<CPUProcesses;i++){
        int arrivalTime = next_exp(0);
        int bursts = (int)ceil(drand48()*64);
        printf("CPU-bound process %c: arrival time %dms; %d CPU bursts:\n",(char)(i+processes-CPUProcesses+65),arrivalTime,bursts);
        for(int j =0;j<bursts;j++){
            int CPUBurst = next_exp(1)*4;
            if(j!=bursts-1){
                int IOBurst = next_exp(1)*10/8;
                printf("--> CPU burst %dms --> I/O burst %dms\n",CPUBurst,IOBurst);
            }
            else{
                printf("--> CPU burst %dms\n",CPUBurst);
            }

        }
    }
}