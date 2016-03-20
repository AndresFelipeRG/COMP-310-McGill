#include "common.h"

int fd;
Shared* shared_mem;

int setup_shared_memory(){
    fd = shm_open(MY_SHM, O_RDWR, 0666);
    if(fd == -1){
        printf("shm_open() failed\n");
        exit(1);
    }
}

int attach_shared_memory(){
    shared_mem = (Shared*) mmap(NULL, sizeof(Shared), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(shared_mem == MAP_FAILED){
        printf("mmap() failed\n");
        fflush(stdout);
        exit(1);
    }

    return 0;
}

int main(int argc, char *argv[]) {
    sleep(0.5);
    setup_shared_memory();
    attach_shared_memory();
  
    
   if (strcmp(argv[0], "exit") == 0 || argv[0] == NULL){  /* is it an "exit"?     */
               exit(0);     }
   if(argc < 3){

      printf("Please enter more arguments: ./client id job_duration number_of_pages\n");

      exit(1);
   }
   //get job params
    int num_args = argc;
    int id = atoi(argv[1]);
    int pages = atoi(argv[2]);
//    int times = atoi(argv[3]);
    int waiting = -1;
    int res = sem_getvalue(&shared_mem->empty,&waiting);
    if(waiting == 0){
         sleep(1.0);
         printf("\nClient %d, has %d page(s) to print,buffer full,sleeps.\n",id,  pages);
         }

    sem_wait(&shared_mem->empty);
    
    
    sem_wait(&shared_mem->binary);
    
   
  
    int j = shared_mem->number;
 
    int i;

    for(j = shared_mem->number; j< shared_mem->length;j = (j+1)%shared_mem->length){
    //create a job and plaace a job into the sahred buffer.
    if(shared_mem->arr[j][0]<0){
     
        shared_mem->arr[j][0] = pages;
        shared_mem->arr[j][1] = pages;
        shared_mem->arr[j][2] = id;
        shared_mem->arr[j][3] = waiting;
        shared_mem->arr[j][4] = shared_mem->num;
    
        shared_mem->number = (shared_mem->number + 1)%shared_mem->length;
        //shared_mem->number = shared_mem->data+1;
        shared_mem->num = shared_mem->num -1;

         if(waiting == 0){
         printf("\nClient %d wakes up, puts request in Buffer[%d]\n",id,  j);
         }
         if(waiting >0){
        printf("\nClient %d, has %d page(s) to print. puts request in Buffer[%d]\n",id, pages,j);
         }
   
    break;
 
    
    }

    }
   
   
    sem_post(&shared_mem->binary);
    
    
    sem_post(&shared_mem->full);
    
    //release share memory.
  
//    sem_unlink(MY_SHM);
   
   //}
     //   }

    return 0;
}
