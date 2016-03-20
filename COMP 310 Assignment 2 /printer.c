#include "common.h"
#include <unistd.h>

int fd;
int errno;
int MY_LEN ;
Shared* shared_mem;


int setup_shared_memory(){
    fd = shm_open(MY_SHM, O_CREAT | O_RDWR, 0666);
    if(fd == -1){
        printf("shm_open() failed\n");
        exit(1);
    }
    ftruncate(fd, sizeof(Shared) + 6*MY_LEN*sizeof(int));
}

int attach_shared_memory(){
    shared_mem = (Shared*)  mmap(NULL, sizeof(Shared) + 6*MY_LEN*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(shared_mem == MAP_FAILED){
        printf("mmap() failed\n");
        exit(1);
    }
    return 0;
}

int init_shared_memory() {
    shared_mem->length = MY_LEN;
    shared_mem->current = 0;
    shared_mem->data = 0;
    sem_init(&(shared_mem->binary), 1, 1);
    
    sem_init(&(shared_mem->empty),1,MY_LEN);
   
    sem_init(&(shared_mem->full),1,0);
    shared_mem->number = 0;
    shared_mem->num = 0;
}

int main(int argc, char *argv[]) {
  //  printf("Enter the number of slots of the shared buffer: \n");
   // scanf("%d", &MY_LEN);
    if(argc < 2){

      printf("Please enter correctly the arguments like this: ./printer num_slots\n");
    //  fflush(stdin);
      exit(1);
   }
    printf("\nWaiting for clients.\n");
    MY_LEN = atoi(argv[1]);
    sem_unlink(MY_SHM);
    setup_shared_memory();
    attach_shared_memory();
    //init semaphore and shared memory.
    init_shared_memory();
    
    int j= 0;
    int waiting2 = -1;
     for (j=0; j< shared_mem->length; j++) {
        shared_mem->arr[j][0] = -1;
        shared_mem->arr[j][1] = -1;
        shared_mem->arr[j][2] = -1;
        shared_mem->arr[j][3] = -1;
        shared_mem->arr[j][4] = 0;
       
        }

    while (1) {
      
      
        int i = 0;
        int j = 0;
        
        sem_wait(&shared_mem->full);
    
        sem_wait(&shared_mem->binary);
       while(1){
       // for(i = 0; i < shared_mem->length;i++){


            //take_a_job
         if(shared_mem->arr[i][0]>=0 && (shared_mem->arr[i][4] == shared_mem->data)){
         int index= i;
         int times = shared_mem->arr[i][0];
         int id = shared_mem->arr[i][2];
         int pages = shared_mem->arr[i][1];
         int waiting = shared_mem->arr[index][3];
         shared_mem->arr[index][0] = -1; 
         shared_mem->arr[index][2] = -1;
         shared_mem->arr[index][1] = -1;
         shared_mem->arr[index][3] = -1;
         shared_mem->arr[index][4] = 0;
         
         sem_post(&shared_mem->binary);
        
         sem_post(&shared_mem->empty);
         
         //print message
         printf("\nPrinter starts printing job id %d, the job duration is %d, the job has %d page(s) from Buffer[%d].\n", id,times,pages,index);
         //go to sleep
         sleep(pages*1);
         printf("\nPrinter finishes printing job id %d,  %d page(s) printed from Buffer[%d].\n",id, pages, index);
          int empty2 = shared_mem->length;
          sem_getvalue(&shared_mem->empty,&empty2);
        
          shared_mem->current = (shared_mem->current +1)%shared_mem->length;
          shared_mem->data = shared_mem->data-1;

           break;
        
         }
         i = (i+1)%shared_mem->length;
        
         
        }
        
        
  

    }
    sem_unlink(MY_SHM);
    return 0;
}
