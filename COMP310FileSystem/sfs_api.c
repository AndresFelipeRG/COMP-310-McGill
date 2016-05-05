
#include "sfs_api.h"


//sfs_fclose()/sfs_fseek() should return 0 on success and a negative number otherwise

//sfs_fread()/sfs_fwrite() should return the number of bytes read/written.

//So they are not "void" as specified in the handout. Their return type should be //"int"

#define JITS_DISK "sfs_disk.disk"
#define BLOCK_SZ 2048
#define NUM_BLOCKS 120000  //TODO: increase
#define NUM_INODES 193   //TODO: increase //NEEDS TO BE TUNES DO THAT num_entreis_free_blocks is an integer.

// verify that this is right...
#define NUM_INODE_BLOCKS (sizeof(inode_t) * NUM_INODES / BLOCK_SZ + 1) 
#define NUM_FREE_BLOCKS_LIST ((NUM_INODES-1)*(12+ BLOCK_SZ/(sizeof(unsigned int)))) //12 by design and number of data pointers in inode table.Also, 1st data block is for the root.
#define MAX_DATA_BLOCKS_PER_FILE  12+ BLOCK_SZ/(sizeof(unsigned int))
#define NUM_ENTRIES_FREE_BLOCKS NUM_FREE_BLOCKS_LIST/64
#define MAXFILENAME 20
superblock_t sb;
inode_t table[NUM_INODES];
unsigned int num_files = 0;
unsigned int current_file = 0;
int NUMB_BLOCKS_TAKES_BY_ROOT = 0; // number of blocks in the disk taken by the root.
//root directory
directory_table root[NUM_INODES-1];
file_descriptor fdt[NUM_INODES]; // The file descriptor table stores the files that are curently open. includes root
free_block_list  free_blocks[NUM_ENTRIES_FREE_BLOCKS]; // each entry has a number of 64 bits which stores 64 indices . 64bits times 1572 = num free block list assume 0 is taken by root., which is equivalent to 1+ sb.inode_table_len;
void init_superblock() {
    sb.magic = 0xACBD0005;
    sb.block_size = BLOCK_SZ;
    //sb.fs_size = ( NUMB_BLOCKS_TAKES_BY_ROOT+1+NUM_INODE_BLOCKS+NUM_FREE_BLOCKS_LIST)* BLOCK_SZ;
    sb.inode_table_len = NUM_INODE_BLOCKS;
    sb.length_free_block_list = NUM_FREE_BLOCKS_LIST;
    sb.number_free_blocks = (sizeof(free_block_list)*NUM_ENTRIES_FREE_BLOCKS)/BLOCK_SZ +1;//iam going to use uint64 which maps 64 blocks with 8 bytes
    sb.fs_size = ( NUMB_BLOCKS_TAKES_BY_ROOT+1+NUM_INODE_BLOCKS+NUM_FREE_BLOCKS_LIST+sb.number_free_blocks)* BLOCK_SZ;
    
    sb.root_dir_inode = 0; //data block at 0

}

void mksfs(int fresh) {
	//Implement mksfs here
    if (fresh) {	

       // printf("making new file system\n");
        NUMB_BLOCKS_TAKES_BY_ROOT = sizeof(root)/BLOCK_SZ +1;
        // create super block
        init_superblock();

        init_fresh_disk(JITS_DISK, BLOCK_SZ, sb.fs_size/(BLOCK_SZ) + 1);
        int i = 0;
        for(i = 0; i < NUM_INODES; i++){
            table[i].taken = 0;
            fdt[i].inode = -1;//inode must be greater than 0 so 0  means inode fd entry is free.
        }
        for(i = 0; i < NUM_INODES-1; i++){
           root[i].names = "";//inode must be greater than 0 so 0  means inode fd entry is free.
           root[i].mode = 0;
        }
        
        
        /* write super block
         * write to first block, and only take up one block of space
         * pass in sb as a pointer
         */
        write_blocks(0, 1, &sb);
      
      
       //  buff = "lamao";
        // write inode table
        //printf("%s and size is %d bytes stored in %d \n",buff, strlen(buff), 1  + NUM_ENTRIES_FREE_BLOCKS+sb.inode_table_len);
        
        current_file = 0;
        num_files = 0;
        //root[0].names ="felipe1";
        fdt[0].inode = 0; //root takes inode 0.
        fdt[0].rwptr = 0;
        //root.rwptr[0] = 100;
        //make sure taken variable is clean
       
        //for testing purposes delete laTER.
       // table[0].data_ptrs[0] = 1+sb.inode_table_len; //store data blocks position
        table[0].taken =1; //means that the inode is taken, if not it is 0.

       // table[1].data_ptrs[0]  = 1+sb.inode_table_len+NUMB_BLOCKS_TAKES_BY_ROOT;//after inode table 1st data block is for the rootstore position from memory table.
        //table[1].taken = 1; //means that the inode is taken, if not it is 0.
        //free_blocks[0].num = 0;
       // NUMB_BLOCKS_TAKES_BY_ROOT = sizeof(root)/BLOCK_SZ +1; /// or sizeof(directory_table)*(NUM_INODES-1)/BLOCK_SZ +1
        uint64_t n = 1;
       
        free_blocks[0].num |= n<<63; //ONLY 1 FILE, REMEMBER ROOT IS TAKEM BY DEFAULT
       // uint64_t n2 = 1;
        //free_blocks[0].num =  free_blocks[0].num |(n2<<62);
        //printf("Block list first entry %lu\n", free_blocks[0].num );
        write_blocks(1, NUM_INODE_BLOCKS, table);
        write_blocks(1+NUM_INODE_BLOCKS,sizeof(directory_table)*(NUM_INODES-1)/BLOCK_SZ +1, root);//blocks in memory sizeof(directory_table)*(NUM_INODES-1)/BLOCK_SZ +1
        //write_blocks(1+sb.inode_table_len+NUMB_BLOCKS_TAKES_BY_ROOT,1,(void*)buff );
        ///printf(" size of inode  %d bytes\n",sizeof(inode_t));
        //printf("number blocks for inode table %d\n", NUM_INODE_BLOCKS);
        //printf(" size of root %d bytes\n",sizeof(root));
       
        //printf("number blocks for root %d\n",sizeof(directory_table)*(NUM_INODES-1)/BLOCK_SZ +1);
        //printf("number blocks taken in memory for free list %d\n", (sb.number_free_blocks));
        //printf("Number of data blocks is %d\n", NUM_FREE_BLOCKS_LIST);
     
        write_blocks(1+NUM_INODE_BLOCKS+NUM_FREE_BLOCKS_LIST+ NUMB_BLOCKS_TAKES_BY_ROOT,(sizeof(free_block_list)*NUM_ENTRIES_FREE_BLOCKS)/BLOCK_SZ +1,free_blocks);
      
        //printf("Beginning of data blocks %d\n",1+sb.inode_table_len+NUMB_BLOCKS_TAKES_BY_ROOT);
        //printf("Beginning of free list %d\n", 1+NUM_INODE_BLOCKS+NUM_FREE_BLOCKS_LIST+NUMB_BLOCKS_TAKES_BY_ROOT);
       // printf("Number of free blocks %s\n", NU);
        //printf("end\n");
        fflush(stdout);
       
        //return;
        // write free block list
        
    


    } else {
        //printf("reopening file system\n");
        int i = 0;
        for(i = 0; i < NUM_INODES; i++){
            
            fdt[i].inode = -1;//inode must be greater than 0 so 0  means inode fd entry is free.
            fdt[i].rwptr = 0;
        }
        current_file = 0;
        //num_files = 1;
        // open super block
        read_blocks(0, 1, &sb);
        NUMB_BLOCKS_TAKES_BY_ROOT = sizeof(root)/BLOCK_SZ +1;
        //printf("Block Size is: %lu bytes\n", sb.block_size);
        read_blocks(1, NUM_INODE_BLOCKS, table);
        read_blocks(1+NUM_INODE_BLOCKS,sizeof(directory_table)*(NUM_INODES-1)/BLOCK_SZ +1, root);//blocks in memory sizeof(directory_table)*(NUM_INODES-1)/BLOCK_SZ +1
        //write_blocks(1+sb.inode_table_len+NUMB_BLOCKS_TAKES_BY_ROOT,1,(void*)buff );
     
        read_blocks(1+NUM_INODE_BLOCKS+NUM_FREE_BLOCKS_LIST+ NUMB_BLOCKS_TAKES_BY_ROOT, (sizeof(free_block_list)*NUM_ENTRIES_FREE_BLOCKS)/BLOCK_SZ +1,free_blocks);
      
        //printf("Beginning of data blocks %d\n",1+NUM_INODE_BLOCKS+NUMB_BLOCKS_TAKES_BY_ROOT);
        //printf("Beginning of free list %d\n", 1+NUM_INODE_BLOCKS+NUM_FREE_BLOCKS_LIST+NUMB_BLOCKS_TAKES_BY_ROOT);
        // open inode table
      //  read_blocks(1, sb.inode_table_len, table);
       
        // open root directory
       // read_blocks(1+sb.inode_table_len,sizeof(root)/BLOCK_SZ +1, root); //120 maximum files
        //open free list
       // read_blocks(1+sb.inode_table_len+NUM_FREE_BLOCKS_LIST +  NUMB_BLOCKS_TAKES_BY_ROOT, sb.number_free_blocks, free_blocks);
        ///printf("Beginning of data blocks %d\n",1+NUM_INODE_BLOCKS+NUMB_BLOCKS_TAKES_BY_ROOT);
        //printf("Beginning of free list %d\n", 1+NUM_INODE_BLOCKS+NUM_FREE_BLOCKS_LIST+NUMB_BLOCKS_TAKES_BY_ROOT);
        i = 0;
        num_files = 0;
        for(i = 1; i < NUM_INODES ;i++){ //skip first inode since it is the directory
            if(table[i].taken == 1){
                num_files = num_files+1;
            }
        }
        //printf("Setting the file directory table with %d files\n",num_files);
        //printf("Number of data blocks is %d\n", NUM_FREE_BLOCKS_LIST);
     
        //int i = 0;
        //for(i = 0; i < num_files; i++){
        //    if(strcmp("", root.names[i]) != 0 && root.names[i]!= NULL){
          //        fdt[i].inode = i;
           //        fdt[i].rwptr = root.rwptr[i];

          //  }   
       // }
    }
	return;
}

int sfs_getnextfilename(char *fname) {
         //question do I have to store the current position every time?Even in the disk, or set it to the first element in the directory table every time mkfs is called?
	//Implement sfs_getnextfilename here
    //fname = (char*) malloc(20);
     //read_blocks(1+NUM_INODE_BLOCKS,sizeof(directory_table)*(NUM_INODES-1)/BLOCK_SZ +1, root);
     int k = 0;
     unsigned int * current_file1 = &current_file;
  //   file_descriptor* f = &fdt[*current_file1];
     num_files = 0;
        for(k = 1; k < NUM_INODES ;k++){ //skip first inode since it is the directory
            if(table[k].taken == 1){
                num_files = num_files+1;
            }
        }

    // int i = 0;
    //int num_files_opened = 0;
      //  for(i = 1; i < NUM_INODES ;i++){ //skip first inode since it is the directory
        //    if(fdt[current_file].inode > 0){
          //      num_files_opened = num_files_opened +1;
            //}
        //}
    if(num_files== 0){*current_file1 = 0; current_file = *current_file1; return 0;}
   // fprintf(stderr, "NUM FILES %d inode %d CURRENT FILE%d and file is %s\n",num_files_opened , fdt[current_file].inode, current_file,root[current_file].names );
    //fprintf(stderr, "NUM FILES in memory %d\n",num_files );
    //if(current_file <= num_files){
    // fprintf(stderr, "NUM FILES %d inode %d CURRENT FILE%d and file is %s\n",num_files , fdt[current_file].inode, current_file,root[current_file].names );
    //}
    //inode_t table_copy[NUM_INODES];
    //printf("Inode root is %d\n", sb.root_dir_inode);
    //index of data block with directory table.
    //unsigned int index = table[sb.root_dir_inode].data_ptrs[0];
    //printf("Index of data block with table OR directory of files is %d\n", index);
    //printf("Current file index is %d\n", current_file);
    ///printf("NUmber of files is %d\n", num_files_opened );
    //if(current_file<num_files && current_file >= 0){
     // fname = root[current_file].names;

      
       int y = 0;
       //int size = 0;
     //  FILE * fp;

       //fp = fopen ("/home/2014/arinco1/file2.txt", "a+");
    
        
       
       int o = 0;
     /*  if(root[current_file].mode == 0 ){
         *current_file1 = *current_file1 +1;
          current_file = *current_file1;
          fprintf(fp, " i am weird %d\n", current_file);
          fclose(fp);
          return 1; 
       }
       while(root[current_file].mode == 0 ){
         *current_file1 = *current_file1 +1;
          current_file = *current_file1; 
              fprintf(fp, " changing and current_file is %d\n", current_file);
          if(current_file>= num_files){
             *current_file1 = 0;
              current_file = 0;
               fprintf(fp, " reached end and current_file is %d\n", current_file);
     
               return 0;
          }

        }*/
       int iterate = 0;
       int visited = 0;
       for(iterate = 0; iterate < NUM_INODES-1; iterate++){
          if(root[iterate].mode == 1){
            if(visited == current_file){
                      
                while(root[iterate].names[o]!= '\0' || root[iterate].names[o]!= '\0'){o = o+1;}

                 for(y = 0; y < o; y++){

                // fprintf(fp, " current file %d inode is %d and num files is %d path %s vs name %c  \n",current_file,fdt[iterate].inode, num_files,fname,root[iterate].names[y]);
          //fclose(fp);
                 fname[y] = root[iterate].names[y];
                
                    }
                  *current_file1 = *current_file1 +1;
                  current_file = *current_file1;

                  fname[y] = '\0';
                 // fclose(fp);
                 // if(*current_file1 == NUM_INODES-1){
                   /// *current_file1 = 0;
                   //current_file = 0;
        
                 //return 1;
                       //}
                  return 1;

           
            }
           visited = visited+1;
          }
       }
     
                   *current_file1 = 0;
                   current_file = *current_file1;

    //fclose(fp);
      
      // fprintf(stderr, "PASSED THIS %s vs first file %svs last file %s\n",filename, root[0].names, root[num_files-1].names);
      //fname = root[current_file].names;
     //memcpy(fname,root.names[root.current_file], sizeof(root.names[root.current_file]));
    // printf("Current files is %s\n", fname);
    // current_file++;}

    // *current_file1 = *current_file1 +1;
    // current_file = *current_file1;
    // if(current_file< num_files){
     //write_blocks(1+sb.inode_table_len,sizeof(root)/BLOCK_SZ +1, root);
     //return 1;}

     //return 1;
  // }
   /* if(current_file>= num_files){
        *current_file1 = 0;
        current_file = 0;
     
        return 0;
    //}*/
   
    
	return 0;
}


int sfs_getfilesize(const char* path) {
    //assuming for each file and end of character is placed and it is null
	//Implement sfs_getfilesize here	
  //  FILE * fp;

   // fp = fopen ("/home/2014/arinco1/file.txt", "a+");
     //fclose(fp);
    
    int size = 0;
    //printf("Calling sfs_getfilesize\n");
    //int beginning_of_data_blocks = 1+sb.inode_table_len +1;
    //printf("Beginning of data blocks for files is %d\n", beginning_of_data_blocks);
    int k = 0;
    num_files = 0;
        for(k = 1; k < NUM_INODES ;k++){ //skip first inode since it is the directory
            if(table[k].taken == 1){
                num_files = num_files+1;
            }
        }

    
    int index_data_block = -1;
    int i = 0;
    for(i = 0; i < NUM_INODES-1; i++){
       if(strcmp(path,root[i].names) == 0){
        //since inode at 0 corresponds to the root.
      //  fprintf(fp, " current file %d and i is %d path %s vs name %s  \n",current_file, i,path,root[i].names);
       //  fclose(fp);
       // int size2 = sizeof(table[i+1]);
        if(table[i+1].size >0){
            size = size + table[i+1].size;
        }
        int counter2 = 0;
        for(counter2 = 0; counter2 < 12; counter2++){
            index_data_block = table[i+1].data_ptrs[counter2]; // data pointers must be greater than 0
            if(index_data_block > 0){
            //printf("Index data block file %d\n", index_data_block);
            char  buff_data[BLOCK_SZ]; //avoid the stack smashing with same size.
            //printf("size of the buffer is %d\n", strlen(buff_data));
            read_blocks(index_data_block,1, (void*)buff_data);
            int k = 0;
            for(k = 0; k< BLOCK_SZ; k++){
               if(buff_data[k] != '\0'){
                size +=1;
              //  printf("%c ", buff_data[k]);
               }
            }

            //printf("size of the data at block %d is %d bytes buffer is %s of length %d\n",index_data_block,strlen(buff_data), buff_data, strlen(buff_data));
           // size  = size + strlen(buff_data);
            //printf("%d\n", buff_data[0]>0);
            //printf("%d\n", buff_data[1] >0);
            //printf("%d\n", buff_data[2] == NULL);//need to make sure there is a null termiantor at the end of the file
           
            }
            index_data_block = -1;
             //read_blocks(block, 1, (void*) buf);
        }
        //assuming each data block is 2048 bytes and each data pointer is 4 bytes long, so there are 512 data blocks available from the inderect data pointer.
        int counter3 = 0;
        for(counter3 = 0; counter3 < MAX_DATA_BLOCKS_PER_FILE-12; counter3 ++){
           index_data_block = table[i+1].indirect[counter3];//remember there are only NUM_INODES -1 files, so have to add 1 to find correct place in inode table 
           ///try storing at the end of the inode table and the table of names. 
           if(index_data_block >0){
           // printf("Index data block file %d\n", index_data_block);
            char  buff_data[BLOCK_SZ]; //avoid the stack smashing with same size.
          //  printf("size of the buffer is %d\n", strlen(buff_data));
            read_blocks(index_data_block,1, (void*)buff_data);//buff data can be of 1024 or 2048
             int k = 0;
            for(k = 0; k< BLOCK_SZ; k++){
               if(buff_data[k] != '\0'){
                size +=1;
                   // printf("%c ", buff_data[k]);
               }
            }
           // int size_data_block = strlen(buff_data);
            //printf("size of the data at block %d is %d bytes buffer is %s of length %d\n",index_data_block,strlen(buff_data), buff_data, strlen(buff_data));
            //size  = size + strlen(buff_data);
           }
           index_data_block = -1;
        }
        
        break;

       }
    }
    //if(path[0] == '/'){return size}
	return size;
}

int sfs_fopen(char *name) {
    //DONT FORGET TO GIVE SAME FILE DESCRITOR ENTRY IF IT IS ALREADY OPEN.
    //by default, maximum file size is 20.
    //read_blocks(1, NUM_INODE_BLOCKS, table);
    //read_blocks(1+NUM_INODE_BLOCKS,sizeof(directory_table)*(NUM_INODES-1)/BLOCK_SZ +1, root);
    // FILE * fp;

   // fp = fopen ("/home/2014/arinco1/file3.txt", "a+");
      int k = 0;
    num_files = 0;
        for(k = 1; k < NUM_INODES ;k++){ //skip first inode since it is the directory
            if(table[k].taken == 1){
                num_files = num_files+1;
            }
        }

    int o2 = 0;
    while(name[o2]!= '\0' || name[o2]!= '\0'){o2 = o2+1;}
    if(o2> 20){
        return -1;
    }
     uint64_t  p = 0;
     for(p = 0; p < NUM_INODES-1; p++){

       if(strcmp(name,root[p].names) == 0){
             //verify if the file is already opened.
      //  printf("Trying to open file already in file descriptor table\n");
            int m = 0;
            int inode_num = p+1; //num files = num inodes -1
            for(m = 0; m < NUM_INODES; m++){
                  file_descriptor* f = &fdt[m];
               if(f->inode == inode_num){
                //printf("Trying to open file already in file descriptor table\n");
                  return -1; //return already allocated entry index.
               }
            }
       }

   }
   // read_blocks(1+sb.inode_table_len,sizeof(directory_table)*(NUM_INODES-1)/BLOCK_SZ +1, root);
    //directory_table  root2 ;
  
    
    int inode_table_position = -1;
   //int num_files = root.data[1];
  //  printf("Calling sfs_fopen. Trying to open %s and the number of files is %d\n",name, num_files);
    uint64_t  i = 0;
    for(i = 0; i < NUM_INODES-1; i++){

       if(strcmp(name,root[i].names) == 0){
         inode_table_position = i+1; //the file already exists in memory.
       //  printf("The inode table position of the file %s is %d\n",  name, inode_table_position);
         //verify if the file was already open, if not return a negative number.?
         uint64_t j = 0;
         for(j = 1; j < NUM_INODES ; j++){
           //   file_descriptor* f = &fdt[j];
            //inode must be bigger than 0, inode 0 is the root. 
            if(fdt[j].inode == -1){
                fdt[j].inode = inode_table_position;

                fdt[j].rwptr = sfs_getfilesize(name); //place the rwptr at the end of the file
              //   printf("The file pointer rwptr is %d\n", f->rwptr);
                //fdt[j].inode = f->inode;
                //fdt[j].rwptr = f->rwptr;
                root[i].mode = 1;
                table[inode_table_position].taken = 1;
               // fprintf(stderr, "ERROR: FILE INODE %s %d and descriptor \n", root[i].names, inode_table_position);
              //  write_blocks(1,sb.inode_table_len, table);
                ///table[inode_table_position]; copy on root disk and table
               if(inode_table_position <0) {
               // printf("Something wrong happened, so the file descriptor is negative instead of positive.\n");
                return -1;
               }
                return j; //file descriptor from 0 to NUM_INODES-1;
            }
         }
       }
   }
   //If we are here it means that the file does not exist.
   //Need to find a free inode, how?
   for(i = 1; i < NUM_INODES ; i++){
       if(table[i].taken == 0){
           inode_table_position = i ; //assuming that the root is a 0 and is taken by default. remember to read from inodes need to offset of 1 from the superblock.
            //  printf("Inode table position for file %s is %d\n", name, inode_table_position );
             if(inode_table_position >= 0){ 
               uint64_t j = 0;
                for(j = 1; j< NUM_INODES; j++){
                      file_descriptor* f = &fdt[j];
                   if(f->inode == -1){
                     f->inode = inode_table_position;
                     f->rwptr = 0;
                      //file is created so the position is 0.
                     num_files++; //new file created.
                     table[inode_table_position].taken = 1;
                     table[inode_table_position].mode = 1;
                     //fprintf(stderr, "%s\n", );
                     
                     int o = 0;
                     while(name[o]!= '\0' || name[o]!= '\0'){o = o+1;}
                   //   fprintf(stderr, " sizeof %d vs strlen %d \n", o, strlen(name) );
                     int min = 0;
                      int length =o;
                      root[inode_table_position-1].names = (char*) malloc(length);
                    
                     for(min = 0; min < length; min++){
                        root[inode_table_position-1].names[min] = name[min];
                     }
                     root[inode_table_position-1].names[min] = '\0';
                     root[inode_table_position-1].mode = 1;
                     //strncpy(root[inode_table_position-1].names, name, strlen(name));
                 
                    //  fprintf(fp, " path %s vs name %s root position %d and inode is %d\n",name,root[inode_table_position-1].names,inode_table_position-1, inode_table_position);
                     //fclose(fp);
                  //  strcpy(root[inode_table_position-1].names,name);//verify name is set properly
                    // printf("Name at positiion %d is %s and number of files is %d\n",table[inode_table_position].taken, root[inode_table_position-1].names, num_files);
                     write_blocks(1+NUM_INODE_BLOCKS,sizeof(directory_table)*(NUM_INODES-1)/BLOCK_SZ +1, root);
                     write_blocks(1,NUM_INODE_BLOCKS, table);
                     //copy on disk root and table.          
                     return j;
                  }
                }
            
             }
       }
   }
   return -1; 
}

int sfs_fclose(int fileID){
   
	//Implement sfs_fclose here	
    //make sure to change 
    //int i = 0;
    //for(i = 0; i< NUM_INODES; i++){
   //printf("Calling closing method \n");
    if(fileID > 0 && fileID < NUM_INODES ){
        file_descriptor* f = &fdt[fileID];
     //   printf("INODE INODE INODE %d is \n", f->inode);
        if(f->inode == -1){
            return -1; //already closed file
        }
        f->inode = -1;
        f->rwptr= 0;
        fdt[fileID].rwptr = 0;//check this with printf
       // printf("Now rwptr is at %d\n", fdt[fileID].rwptr);
        fdt[fileID].inode = -1;//check this with printf
        //printf("Now inode for %d is  %d\n",fileID, fdt[fileID].inode);

    return 0;
    }
        
	return -1;
}

int sfs_fread(int fileID, char *buf, int length){

	//Implement sfs_fread here	
    int k = 0;
    num_files = 0;
        for(k = 1; k < NUM_INODES ;k++){ //skip first inode since it is the directory
            if(table[k].taken == 1){
                num_files = num_files+1;
            }
        }
    NUMB_BLOCKS_TAKES_BY_ROOT = sizeof(root)/BLOCK_SZ +1;
    int qte_bytes = length;
    int bytes_read = 0;
    int num_pointers_data_blocks_per_file =  12+ BLOCK_SZ/(sizeof(unsigned int));
    file_descriptor* f = &fdt[fileID];
    //printf("id from fopen is %d\n", fileID);
    
    
    int flag_something_was_read = -1;

    //printf("Number of pointers to data blocks per file %d\n",num_pointers_data_blocks_per_file);
 
    if(f->inode == -1){
      //  printf("This file descriptor entry is empty: %d\n", fileID);
        //printf("The entry is inode %d\n",fdt[fileID].inode );
        return 0;
    }
    if(f->inode == 0){
        //printf("Error, you are trying to open the root.\n");
        return 0;

    }
   
   // inode_t* n = &table[f->inode];
    //printf("Calling sfs_fread wants to read %d bytes.\n", length);
    //printf("Inode number is %d\n",f->inode);
    //printf("The rwptr is %d \n",f->rwptr);
    //printf("Want to read to file %s\n", root[f->inode -1].names);
    //int size_file = sfs_getfilesize(root[f->inode -1].names);
    int capacity_file_bytes= BLOCK_SZ*(12 + BLOCK_SZ/sizeof(unsigned));
   // printf("The file capacity is %d bytes\n",capacity_file_bytes );

    //printf("Size of the file  %s is %d bytes\n",root[f->inode -1].names,size_file );
    //how many blocks to  write
   // int num_blocks = length/BLOCK_SZ +1;
    //printf("Needs to read about %d blocks \n",num_blocks);
    //loop through data blocks if necessary and allocate as you iterate. First find from where to write.Nees also to make sure if we are appending there is a null terminator at the end.
    //start writing from 
    //rwptr never goes beyond 2047
    //write contigously from data pointer to data pointer
    int block = f->rwptr/BLOCK_SZ + 1;
    //int counter_blocks= num_blocks;
    int current_block = block-1;//block indexes start from 0.
    if(current_block <0){
      //  printf("ERROR: block to start reading is negative.\n");
        return 0;
    }
    //question about size of disk, number of files, how to verify fuse and file system work and can I say to write more data than file capacity and end up writing just a portion of it. 
    if(f->rwptr >= (BLOCK_SZ*num_pointers_data_blocks_per_file)){
        //printf("error: it is not possible to read in addresses beyond the addres space of a file.\n");
        return 0;
    }
    if(f->rwptr < 0){
        //printf("error: rwptr is negative\n");
        return 0;
    }
    
     //printf("The rwptr is %d Start reading block(index) %d\n",f->rwptr, block-1);
    while (qte_bytes > 0 && (capacity_file_bytes>0) && current_block < (num_pointers_data_blocks_per_file))
    {
       int beginning_withing_block = f->rwptr % BLOCK_SZ;//verify this
    
       char buff[BLOCK_SZ];//2048 BYTES
       //int position_null= 0;
       if(current_block<12){//no need of indirect pointer.
  
    
    
      //check if current block hasnt been allocated before in memory
  
        //place null pointer at the beginning of the buffer
      //  buffer[]
    
      //verify if the d15/20ata block has been allocated before or not. 
           if(table[f->inode].data_ptrs[current_block] > 0){
            //the data block has been allocated before. 
            read_blocks(table[f->inode].data_ptrs[current_block], 1, (void*)buff);
           // fprintf(stderr, "DATA ALLOCATED BEFORE file %sand %d and inode %d\n",root[f->inode -1].names, table[f->inode].data_ptrs[current_block], f->inode );
            //flag_something_was_read = 1
           }

           if(table[f->inode].data_ptrs[current_block] ==0){
            //writing data bocks for the first time
            table[f->inode].data_ptrs[current_block] =0;
            int k = 0;
            for(k = 0; k< BLOCK_SZ; k++){
             buff[k] = '\0'; // evrything to NULL;
          
            }

           }
     
       


       while(current_block < num_pointers_data_blocks_per_file && beginning_withing_block < BLOCK_SZ && qte_bytes >0 && (capacity_file_bytes >0)){



       
      
        buf[bytes_read] = buff[beginning_withing_block] ; ///length
      
        bytes_read+=1;//bytes written so far.
        qte_bytes-=1;//number of bytes remaining to read
         flag_something_was_read = 1;
        
        
          
        capacity_file_bytes-=1;//total capacity of file
        beginning_withing_block+=1;
        f->rwptr+=1;
      

         }


         //IF FINISHED WRITING WHITHING A BLOCK.

      //at this point block of 2048 has been written.
       //set up null pointer how.
            
    

            }


      if(current_block >=12  && current_block < num_pointers_data_blocks_per_file){
             if(table[f->inode].indirect[current_block-12] > 0){
            //the data block has been allocated before. 
            read_blocks(table[f->inode].indirect[current_block-12] , 1, (void*)buff);
           // flag_something_was_read = 1;
        }
      
            if(table[f->inode].indirect[current_block-12]  ==0){
            table[f->inode].data_ptrs[current_block] =0;

             int k = 0;
             for(k = 0; k< BLOCK_SZ; k++){
              buff[k] = '\0';
          
              }

              }
              
          while(current_block < num_pointers_data_blocks_per_file && beginning_withing_block < BLOCK_SZ && qte_bytes >0 && (capacity_file_bytes >0)){
            
           

       
             
              buf[bytes_read] = buff[beginning_withing_block] ; ///length
             
             flag_something_was_read = 1;

             bytes_read+=1;//bytes written so far.
             qte_bytes-=1;//number of bytes remaining to read
      
              flag_something_was_read = 1;
             
            
             capacity_file_bytes-=1;//total capacity of file
             beginning_withing_block+=1;
             f->rwptr+=1;
            }
        }


      if(current_block >= num_pointers_data_blocks_per_file+1){
       //  printf("Exceeded file capacity.\n");
        //printf("Bytes written %d\n",bytes_read);
        return bytes_read;
        }
       // if(block >=12){
        //this is the single inderect portion.
      //}
       //table[f->inode].indirect[p];
       //if there is already a data block allocated store it back
     
      //WRITING PART
      if(current_block < 12){
         if(current_block  >= num_pointers_data_blocks_per_file+1){
          //    printf("Exceeded file capacity.\n");
            //  printf("Bytes read %d\n",bytes_read);
              return bytes_read;
         }



      }
      if(current_block >=12){
        if(current_block  >= num_pointers_data_blocks_per_file+1){
              ///printf("Exceeded file capacity.\n");
              //printf("Bytes read %d\n",bytes_read);
              return bytes_read;
         }
       
      

}
      

  

  
    current_block+=1;

   }

     


    //fprintf(stderr, "NULLPTRS WRITTER %d vs read bytes %d\n",table[f->inode].size, bytes_read );
    fdt[fileID].rwptr = f->rwptr;//check this with printf
    //printf("Now rwptr is at %d\n", fdt[fileID].rwptr);
    fdt[fileID].inode = f->inode;//check this with printf
    //printf("Now inode for %d is  %d\n",fileID, fdt[fileID].inode);
     int chars = 0;
     int a = 0;
   
    if(table[f->inode].size == 0){
       for(a= 0; a < length; a++){
     if(buf[a] !='\0'){
     chars++;
    }
    }
    return chars;
    }
    if( flag_something_was_read==1){
    return bytes_read;}
    return 0;
}

int sfs_fwrite(int fileID, const char *buf, int length)
   {
    int k = 0;
    num_files = 0;
        for(k = 1; k < NUM_INODES ;k++){ //skip first inode since it is the directory
            if(table[k].taken == 1){
                num_files = num_files+1;
            }
        }

     NUMB_BLOCKS_TAKES_BY_ROOT = sizeof(root)/BLOCK_SZ +1;
     int flag_something_was_written = -1;
     int qte_bytes = length;
     int bytes_written = 0;
     int num_pointers_data_blocks_per_file =  12+ BLOCK_SZ/(sizeof(unsigned int));
     //printf("id from fopen is %d\n", fileID);

     //printf("Number of pointers to data blocks per file %d\n",num_pointers_data_blocks_per_file);
 
     file_descriptor* f = &fdt[fileID];
   // printf("The first entry of free list is %d\n", free_blocks[0].num);
    //check inode number is not -1 or 0.

   
    //first verify that the file descriptor is not -1
    if(f->inode == -1){
       // printf("This file descriptor entry is empty: %d\n", fileID);
        //printf("The entry is inode %d\n",fdt[fileID].inode );
        
        // fprintf(stderr, "F INODE  = -1\n");
        return 0;
    }
    if(f->inode == 0){
        // fprintf(stderr, "F INODE  = 0\n");
        //printf("Error, you are trying to open the root.\n");
        return 0;

    }
    //inode_t* n = &table[f->inode];
    //printf("Calling sfs_write wants to write %d bytes.\n", length);
    //printf("Inode number is %d\n",f->inode);
   // printf("Want to write to file %s\n", root[f->inode -1].names);
    int size_file = sfs_getfilesize(root[f->inode -1].names);
    int capacity_file_bytes= BLOCK_SZ*(12 + BLOCK_SZ/sizeof(unsigned));
    if(size_file >= capacity_file_bytes){
        return 0;
    }
//    printf("The file capacity is %d bytes\n",capacity_file_bytes );

  //  printf("Size of the file  %s is %d bytes\n",root[f->inode -1].names,size_file );
    //where to write.
   // int num_blocks = length/BLOCK_SZ +1;
    //printf("Needs to write %d blocks \n",num_blocks);
    //loop through data blocks if necessary and allocate as you iterate. First find from where to write.Nees also to make sure if we are appending there is a null terminator at the end.
    //start writing from 
    //rwptr never goes beyond 2047
    //write contigously from data pointer to data pointer
    int block = f->rwptr/BLOCK_SZ + 1;
    //int counter_blocks= num_blocks;
    int current_block = block-1;//block indexes start from 0.
    if(current_block <0){
      //  printf("ERROR: block to start writing is negative.\n");
        return 0;
    }
    //question about size of disk, number of files, how to verify fuse and file system work and can I say to write more data than file capacity and end up writing just a portion of it. 
    if(f->rwptr >= (BLOCK_SZ*num_pointers_data_blocks_per_file)){
        //printf("error: it is not possible to write in addresses beyond the addres space of a file.\n");
        return 0;
    }
    if(f->rwptr < 0){
        //printf("error: rwptr is negative\n");
        return 0;
    }
    //printf("The rwptr is %d Start writing in block(index) %d\n",f->rwptr, block-1);
    while (qte_bytes> 0 && (capacity_file_bytes>0) && current_block < (num_pointers_data_blocks_per_file))
    {
       int beginning_withing_block = f->rwptr % BLOCK_SZ;//verify this
    
       char buff[BLOCK_SZ];//2048 BYTES
      // int position_null= 0;
       if(current_block<12){//no need of indirect pointer.
      //printf("HII\n");
    
    
      //check if current block hasnt been allocated before in memory
  
        //place null pointer at the beginning of the buffer
      //  buffer[]
    
      //verify if the data block has been allocated before or not. 
           if(table[f->inode].data_ptrs[current_block] > 0){
            //the data block has been allocated before. 
           //  fprintf(stderr, "OPENING PREVIOUSLY WRITTE BLOCK\n");
            read_blocks(table[f->inode].data_ptrs[current_block], 1, (void*)buff);
            
            // fprintf(stderr, "OPENING PREVIOUSLY WRITTE inode %d rwptr %d current block %d  data %d %s\n",f->inode, f->rwptr, current_block, table[f->inode].data_ptrs[current_block], buff);
              }

           if(table[f->inode].data_ptrs[current_block] ==0){
            //writing data bocks for the first time
           // printf("HII2\n");
    
            table[f->inode].data_ptrs[current_block] =0;
            int k = 0;
            for(k = 0; k< BLOCK_SZ; k++){
             buff[k] ='\0'; //set evrything to NULL;
          
            }

           }
     
       


       while(current_block < num_pointers_data_blocks_per_file && beginning_withing_block < BLOCK_SZ && qte_bytes >0 && (capacity_file_bytes >0)){
         
        if(buf[bytes_written] == '\0'){
            table[f->inode].size = table[f->inode].size +1;
            //fprintf(stderr, "WRITING NULLPTR EXPLICITLY %d\n",table[f->inode].size);
        }
        buff[beginning_withing_block] = buf[bytes_written]; ///length
        beginning_withing_block+=1;
        bytes_written+=1;//bytes written so far.
        qte_bytes-=1;//number of bytes remaining to write
        capacity_file_bytes-=1;//total capacity of file
        f->rwptr+=1;
         }
         //IF FINISHED WRITING WHITHING A BLOCK.

      //at this point block of 2048 has been written.
       //set up null pointer how.
            
    

            }


      if(current_block >=12  && current_block < num_pointers_data_blocks_per_file){
     //


     //   printf("HII\n");
    
             if(table[f->inode].indirect[current_block-12] > 0){
            //the data block has been allocated before. 
            read_blocks(table[f->inode].indirect[current_block-12] , 1, (void*)buff);}
      
            if(table[f->inode].indirect[current_block-12]  ==0){
            table[f->inode].indirect[current_block-12] = 0;

             int k = 0;
             for(k = 0; k< BLOCK_SZ; k++){
              buff[k] = '\0';
          
              }

              }
              
          while(current_block < num_pointers_data_blocks_per_file && beginning_withing_block < BLOCK_SZ && qte_bytes >0 && (capacity_file_bytes >0)){
            
                if(buf[bytes_written] == '\0'){
            table[f->inode].size = table[f->inode].size +1;
           // fprintf(stderr, "WRITING NULLPTR EXPLICITLY \n");
        }
                buff[beginning_withing_block] = buf[bytes_written];
            beginning_withing_block+=1;
             bytes_written+=1;//bytes written so far.
             qte_bytes-=1;//number of bytes remaining to write
           capacity_file_bytes-=1;//total capacity of file
           f->rwptr+=1;
            }
        }
      if(current_block >= num_pointers_data_blocks_per_file +1){
      //   printf("Exceeded file capacity.\n");
        //printf("Bytes written %d\n",bytes_written);
        return bytes_written;
        }
       // if(block >=12){
        //this is the single inderect portion.
      //}
       //table[f->inode].indirect[p];
       //if there is already a data block allocated store it back
     
      //WRITING PART
      if(current_block < 12){
         if(current_block  >= num_pointers_data_blocks_per_file){
          //    printf("Exceeded file capacity.\n");
            //  printf("Bytes written %d\n",bytes_written);
              return bytes_written;
         }
      if(table[f->inode].data_ptrs[current_block] > 0){
        //fprintf(stderr,"Writing into data block at position%d and rwptr is %d\n",table[f->inode].data_ptrs[current_block],f->rwptr);
        write_blocks(table[f->inode].data_ptrs[current_block], 1, (void*)buff);
        flag_something_was_written = 1;

      // int y = 0;
       //for(y = 0; y < 2048; y++){ 
        //if(y == 2047){}
       //fprintf(stderr, "%c", buff[y]);}
        
       }
     if(table[f->inode].data_ptrs[current_block] == 0){
        //need to find a free data block.
      int  pointer_block = 0;
     // int max_data_blocks = NUM_ENTRIES_FREE_BLOCKS;
      int free_data_block = -1;
      for(pointer_block = 0; pointer_block< NUM_ENTRIES_FREE_BLOCKS; pointer_block++){
      uint64_t r = 0;
      uint64_t b = free_blocks[pointer_block].num;
      int i = 0;
      uint64_t j = 1;
      //63 since i am using 64 integers
      for(i = 63 ; i>=0; i--){
       r = 0;
       r |= j<< i;
       if(((r & b) >> i ) == 0){
          //printf("Free data block %d\n", 63-i);
          free_data_block = 63-i + 1 + NUM_INODE_BLOCKS + NUMB_BLOCKS_TAKES_BY_ROOT;
          //printf("data blocks position is %d\n", 63-i + 1 + sb.inode_table_len + NUMB_BLOCKS_TAKES_BY_ROOT);
          uint64_t n2 = 1;
          free_blocks[pointer_block].num =  free_blocks[pointer_block].num |(n2<<i);
          uint64_t r = 0;
         // uint64_t b = free_blocks[pointer_block].num ;
          int k = 0;
           for(k = 63 ; k>=0; k--){
           r = 0;
            r |= j<< k;
           // uint64_t res = (r & b) >> k;
          //  fprintf(stderr,"%d", res );
            }
          break;
       }

       }
       if(free_data_block >=0){
        break; //since a data block was found
       }

         }
        if(free_data_block < 0 ){
            //printf("Unable to write in a data block, all data blocks have been taken%s\n");
            if(flag_something_was_written ==1){
            return bytes_written;}
            return 0;
        }
        if(free_data_block >= 0){
            //printf("Found data block %d\n", free_data_block*(pointer_block+1));
            table[f->inode].data_ptrs[current_block] =  free_data_block + (pointer_block*64);
            //fprintf(stderr,"Writing into data block at position%d\n",table[f->inode].data_ptrs[current_block]);
            write_blocks(table[f->inode].data_ptrs[current_block], 1, (void*)buff);
            write_blocks(1,sb.inode_table_len, table); //write inode table back;
            write_blocks(1+sb.inode_table_len+NUM_FREE_BLOCKS_LIST+ NUMB_BLOCKS_TAKES_BY_ROOT,sb.number_free_blocks,free_blocks);
            flag_something_was_written = 1;
        }
          }



      }
      if(current_block >=12){
        if(current_block  >= num_pointers_data_blocks_per_file){
              //printf("Exceeded file capacity.\n");
              ///printf("Bytes written %d\n",bytes_written);
              return bytes_written;
         }
        if(table[f->inode].indirect[current_block-12] > 0){
        //printf("Writing into data block at position%d\n",table[f->inode].indirect[current_block-12]);
        write_blocks(table[f->inode].indirect[current_block-12], 1, (void*)buff);
        flag_something_was_written = 1;
      //  write_blocks(1,sb.inode_table_len, table); //write inode table back;
      //  write_blocks(1+sb.inode_table_len+NUM_FREE_BLOCKS_LIST+ NUMB_BLOCKS_TAKES_BY_ROOT,sb.number_free_blocks,free_blocks);
       }

          if(table[f->inode].indirect[current_block-12] == 0){
        //need to find a free data block.
      int  pointer_block = 0;
      //int max_data_blocks = NUM_ENTRIES_FREE_BLOCKS;
      int free_data_block = -1;
      for(pointer_block = 0; pointer_block< NUM_ENTRIES_FREE_BLOCKS; pointer_block++){
      uint64_t r = 0;
      uint64_t b = free_blocks[pointer_block].num;
      int i = 0;
      uint64_t j = 1;
      //63 since i am using 64 integers
      for(i = 63 ; i>=0; i--){
       r = 0;
       r |= j<< i;
       if(((r & b) >> i ) == 0){
          //printf("Free data block %d\n", 63-i);
          free_data_block = 63-i + 1 + NUM_INODE_BLOCKS + NUMB_BLOCKS_TAKES_BY_ROOT;
          //printf("data blocks position is %d\n", 63-i + 1 + NUM_INODE_BLOCKS + NUMB_BLOCKS_TAKES_BY_ROOT);
          uint64_t n2 = 1;
          free_blocks[pointer_block].num =  free_blocks[pointer_block].num |(n2<<i);
          uint64_t r = 0;
         // uint64_t b = free_blocks[0].num ;
          int k = 0;
           for(k = 63 ; k>=0; k--){
           r = 0;
            r |= j<< k;
           // uint64_t res = (r & b) >> k;
            //printf("%d", res );
            }
          break;
       }

       }
         if(free_data_block >=0){
        break; //since a data block was found
       }
         }
        if(free_data_block < 0 ){
            //printf("Unable to write in a data block, all data blocks have been taken%s\n");
            if(flag_something_was_written ==1){
            return bytes_written;}
            return 0;
        }
        if(free_data_block >= 0){
           //printf("Found data block %d\n", free_data_block*(pointer_block+1));
            table[f->inode].indirect[current_block-12] = free_data_block + (pointer_block*64);
          //  fprintf(stderr,"Writing into data block at position%d\n",table[f->inode].indirect[current_block-12]);
            write_blocks(table[f->inode].indirect[current_block-12], 1, (void*)buff);
            write_blocks(1,sb.inode_table_len, table); //write inode table back;
            write_blocks(1+sb.inode_table_len+NUM_FREE_BLOCKS_LIST+ NUMB_BLOCKS_TAKES_BY_ROOT,sb.number_free_blocks,free_blocks);
             flag_something_was_written = 1;
        }

          }


}
      

  

  
    current_block+=1;

   }

     
              //  int free_data_block_index = pointer_data_block;
                //


    fdt[fileID].rwptr = f->rwptr;//check this with printf
    //printf("Now rwptr is at %d\n", fdt[fileID].rwptr);
    fdt[fileID].inode = f->inode;//check this with printf
    //printf("Now inode for %d is  %d\n",fileID, fdt[fileID].inode);
    
    
	return bytes_written;
    }

int sfs_fseek(int fileID, int loc){
    file_descriptor* f = &fdt[fileID];
    int capacity_file_bytes= BLOCK_SZ*(12 + BLOCK_SZ/sizeof(unsigned));
    if(loc <  0){
      //  printf("Location in sfs_fseek  %d is out of range %d\n", loc, capacity_file_bytes);
        return -1;
    }
    if(loc >= capacity_file_bytes){
        //printf("Location in sfs_fseek  %d is out of range %d\n", loc, capacity_file_bytes);
        return -1;
    }
   // printf("The first entry of free list is %d\n", free_blocks[0].num);
    //check inode number is not -1 or 0.

   
    //first verify that the file descriptor is not -1
    if(f->inode == -1){
        //printf("This file descriptor entry is empty: %d\n", fileID);
        //printf("The entry is inode %d\n",fdt[fileID].inode );
        return -1;
    }
    if(f->inode == 0){
        //printf("Error, you are trying to open the root.\n");
        return -1;

    }
   // inode_t* n = &table[f->inode];

	//Implement sfs_fseek here	

    /*
     * This is a very minimal implementation of fseek. You should add some type
     * of error checking before you submit this assignment
     */

    fdt[fileID].rwptr = loc;

    
    //printf("Now rwptr is at %d\n", fdt[fileID].rwptr);
    fdt[fileID].inode = f->inode;//check this with printf
    //printf("Now inode for %d is  %d\n",fileID, fdt[fileID].inode);
	return 0;
}

int sfs_remove(char *file) {
     //TABLE[i].taken = 0;
	//Implement sfs_remove here	
    //dont forget to remoe from the file descriptor table.
   // FILE * fp;
    int removed = 0;
    int special_case = 0;
    //fp = fopen ("/home/2014/arinco1/file4.txt", "a+");
     //fprintf(fp, " trying to remove %s\n",file);
    // removed = 1;
   // fclose(fp);
    int k2 = 0;
    num_files = 0;
        for(k2 = 1; k2 < NUM_INODES ;k2++){ //skip first inode since it is the directory
            if(table[k2].taken == 1){
                num_files = num_files+1;
            }
        }
        
    //printf("Calling remove \n");
   //          fprintf(stderr, "TRYING TO REMOVE THE file %s and number of files %d\n", file, num_files);
    int num_pointers_data_blocks_per_file =  12+ BLOCK_SZ/(sizeof(unsigned int));
    int j2;
   // file_descriptor* f = &fdt[fileID];
    for(j2 = 0; j2 < NUM_INODES-1; j2++){
          
          //search file descriptor table
         int o = 0;
        while(root[j2].names[o]!= '\0' || root[j2].names[o]!= '\0'){o = o+1;}
      //  fprintf(fp, " checking this file %s \n",root[j2].names);
        if(root[j2].names[0] == '/'){
          special_case =1;
        //  fprintf(fp, " special_case");
          int iter = 1;
          for(iter = 1; iter < o;iter++){
            if(root[j2].names[iter] != file[iter-1]){special_case = 0; break;}
          }
         
        }
        //removed = j2+1;
        if(strcmp(root[j2].names, file) == 0 || special_case){
    //    fprintf(fp, " in the loop 2 %s and removed %d and inode is %d \n",file, removed, j2+1);
        root[j2].mode = 0;
        //file_descriptor* f = &fdt[j2+1];
        int u = 0;
        int index_file_desc = -1;
        int remove_inode = j2+1;
      //  int *cr = &current_file;
       // *cr = 0;
        current_file = 0;
        
        // fprintf(stderr, "TRYING TO REMOVE THE file %s and inode %d\n", root[j2].names, remove_inode);
        //fprintf(stderr, "TRYING TO REMOVE INODE %d\n", rem);
        for(u = 1; u < NUM_INODES; u++){
            if(fdt[u].inode == remove_inode){
                index_file_desc = u;
                removed = index_file_desc;
              //  fprintf(fp, " in the loop %s and removed %d\n",file, removed);
                break;
            }
        }

        if(index_file_desc == 0){
           //fprintf(stderr, "TRYING TO REMOVE THE ROOT \n");
            return 0;
        }
        if(index_file_desc>0){
          //fprintf(stderr, "TRYING TO REMOVE %d\n", u);
       // sfs_fclose(index_file_desc);
    }
        if(remove_inode > 0){

            
            // printf("Inode root%d vs Inode from fdt %d\n",j2+1, remove_inode );
            
             //free data pointers 
            int k = 0;
            for(k = 0; k < 12; k++){
              
             if(table[remove_inode].taken == 1){
                 
                 char buffer[BLOCK_SZ];
                if(table[remove_inode].data_ptrs[k] > 0){ //this means that a data block has been allocated.
              //       printf("here4 data block %d \n", table[remove_inode].data_ptrs[k]);
                int data_block_position =  table[remove_inode].data_ptrs[k];
               

                int p = 0;
                if(data_block_position > 0){
              for(p = 0; p < BLOCK_SZ; p++){

                  buffer[p] = '\0';
               }
                 //need to free data block list
                 //free_data_block = 63-i + 1 + sb.inode_table_len + NUMB_BLOCKS_TAKES_BY_ROOT;
                
                //("data block index within free_block %d \n", free_data_block_index);
               //1 + NUM_INODE_BLOCKS + NUMB_BLOCKS_TAKES_BY_ROOT 
                int pointer_data_block = -(data_block_position-1-NUM_INODE_BLOCKS-NUMB_BLOCKS_TAKES_BY_ROOT -63 );
                int index_free_block = pointer_data_block/64;
                int free_data_block_index = pointer_data_block%64;
                // free_data_block_index = -((data_block_position-(pointer_data_block*64))  - 1 - 63 - NUM_INODE_BLOCKS- NUMB_BLOCKS_TAKES_BY_ROOT);
              //  fprintf(stderr, "TRYING TO REMOVE  pointer_block %d index free_block %dand free data block index within 64  %d\n", data_block_position,index_free_block,free_data_block_index);
               if(free_data_block_index < 0){
                 //   printf("FATAL ERROR WHEN TRYING TO FREE DATA BLOCK FROM FILE %s\n", file );
                   // return -1;
                 }
                  
                  //need to find a data block and free it.
                         int  pointer_block = index_free_block ;
                         //int max_data_blocks = NUM_ENTRIES_FREE_BLOCKS;
                         
                         // for(pointer_block = 0; pointer_block<NUM_ENTRIES_FREE_BLOCKS; pointer_block++){
                            uint64_t r = 0;
                            uint64_t b = free_blocks[index_free_block].num;
                            int i = 0;
                            uint64_t j = 1;
                            //63 since i am using 64 integers
                             for(i = 63 ; i>=0; i--){
                                 if(i == free_data_block_index){


                                 r = 0;
                                 r |= j<< i;
                                 if(((r & b) >> i ) == 1){
                                     //printf("Freeing data block %d\n", 63-i);
                                
                                    // printf("data blocks position is %d\n", 63-i + 1 +NUM_INODE_BLOCKS + NUMB_BLOCKS_TAKES_BY_ROOT);
                                     uint64_t n2 = 1;
                                     free_blocks[pointer_block].num =  free_blocks[pointer_block].num ^ (n2<<i); //EXCLUSIVE OR
                                    // uint64_t b = free_blocks[pointer_block].num ;
                                     int k3 = 0;
                                     for(k3 = 63 ; k3>=0; k3--){
                                     r = 0;
                                     r |= j<< k3;
                                     //uint64_t res = (r & b) >> k3;
                                     //printf("%d", res );
                                              }
                                     break;
                                 }

                         }
                        }
                     // }


              
                 write_blocks(table[remove_inode].data_ptrs[k], 1, (void*)buffer);
                 table[remove_inode].data_ptrs[k] = 0; //once finished set the data block back to 0
                  table[remove_inode].size = 0;
                 //table[remove_inode].taken == 0;
              }
          }
                 
                

             
              }
            }







                  for(k = 0; k < num_pointers_data_blocks_per_file-12; k++){
              
             if(table[remove_inode].taken == 1){
                 
                 char buffer[BLOCK_SZ];
                if(table[remove_inode].indirect[k] > 0){ //this means that a data block has been allocated.
                     //printf("here4 data block %d \n", table[remove_inode].indirect[k]);
                int data_block_position =  table[remove_inode].indirect[k];
               
                int pointer_data_block = -(data_block_position-1-NUM_INODE_BLOCKS-NUMB_BLOCKS_TAKES_BY_ROOT -63 );
                int index_free_block = pointer_data_block/64;
                int free_data_block_index = pointer_data_block%64;
                int p = 0;
              for(p = 0; p < BLOCK_SZ; p++){

                  buffer[p] = '\0';
               }
                 //need to free data block list
                 //free_data_block = 63-i + 1 + sb.inode_table_len + NUMB_BLOCKS_TAKES_BY_ROOT;
                
                  
                  //need to find a data block and free it.
                       int  pointer_block = index_free_block ;
                        // int max_data_blocks = NUM_ENTRIES_FREE_BLOCKS;
                         
                         // for(pointer_block = 0; pointer_block<NUM_ENTRIES_FREE_BLOCKS; pointer_block++){
                            uint64_t r = 0;
                            uint64_t b = free_blocks[index_free_block].num;
                            int i = 0;
                            uint64_t j = 1;
                            //63 since i am using 64 integers
                             for(i = 63 ; i>=0; i--){
                                 if(i == free_data_block_index){


                                 r = 0;
                                 r |= j<< i;
                                 if(((r & b) >> i ) == 1){
                                     //printf("Freeing data block %d\n", 63-i);
                                
                       //              printf("data blocks position is %d\n", 63-i + 1 +NUM_INODE_BLOCKS + NUMB_BLOCKS_TAKES_BY_ROOT);
                                     uint64_t n2 = 1;
                                     free_blocks[pointer_block].num =  free_blocks[pointer_block].num ^ (n2<<i); //EXCLUSIVE OR
                                    // uint64_t b = free_blocks[pointer_block].num ;
                                     int k3 = 0;
                                     for(k3 = 63 ; k3>=0; k3--){
                                     r = 0;
                                     r |= j<< k3;
                                   //  uint64_t res = (r & b) >> k3;
                         //            printf("%d", res );
                                              }
                                     break;
                                 }

                         }
                        }

                 
                 write_blocks(table[remove_inode].indirect[k], 1, (void*)buffer);
                 table[remove_inode].indirect[k] = 0; //once finished set the data block back to 0
                  table[remove_inode].size = 0;
                // table[remove_inode].taken == 0;
              }
                 
                

             
              }
            }

         
            //since something was removed fix the table and root.
 
              
              table[remove_inode].taken = 0;
              table[remove_inode].size = 0;
              write_blocks(1,NUM_INODE_BLOCKS, table);
         

         } 
       //char names2[20];
       //table[inode_table_position].taken = 1;
                    
       //root[inode_table_position-1].names = name;
       root[j2].names = (char*)malloc(MAXFILENAME);
       root[j2].names[0] = '\0';
      // removed = 1;
      // fprintf(stderr, "TAKEN INODE %d for file %s\n", table[remove_inode].taken, file);
       write_blocks(1,NUM_INODE_BLOCKS, table);
       write_blocks(1+NUM_INODE_BLOCKS,sizeof(directory_table)*(NUM_INODES-1)/BLOCK_SZ +1, root);
       break;
    }
  }
//write_blocks(1+sb.inode_table_len+NUM_FREE_BLOCKS_LIST+ NUMB_BLOCKS_TAKES_BY_ROOT,sb.number_free_blocks,free_blocks);
  //  printf("File does not exist anymore hha. %s\n", file);
  

     write_blocks(1+NUM_INODE_BLOCKS+NUM_FREE_BLOCKS_LIST+ NUMB_BLOCKS_TAKES_BY_ROOT,sb.number_free_blocks,free_blocks);
	 write_blocks(1+NUM_INODE_BLOCKS,sizeof(directory_table)*(NUM_INODES-1)/BLOCK_SZ +1, root);
   //  fprintf(fp, " removed value %d\n",removed);
    //int* current_file1 = &current_file;
    //*current_file1 = 0;
    //current_file = 0;
   // fclose(fp);
    if(removed > 0){return removed;}
    return -1;
}
