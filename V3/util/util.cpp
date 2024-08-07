#include "util.h"
#include <stdio.h>
#include <stdlib.h>

void perror_if(bool condition, const char* errorMessage){

    if(condition){
        perror(errorMessage);
        exit(1);
    }
}