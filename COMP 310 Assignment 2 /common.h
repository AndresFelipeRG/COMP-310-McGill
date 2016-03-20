#ifndef _INCLUDE_COMMON_H_
#define _INCLUDE_COMMON_H_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

// from `man shm_open`
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#include <semaphore.h>

#define MY_SHM "/andfel16"

typedef struct {
	int length;
   
	int current;
    sem_t binary;
    sem_t empty;
    sem_t full;
   
    int number;    
    int data;

    int num;
    int arr[0][5];
} Shared;

#endif //_INCLUDE_COMMON_H_
