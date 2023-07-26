#include <math.h>

#include "header.h"


//0 is floor 1 is ceil
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
    return toReturn;
}