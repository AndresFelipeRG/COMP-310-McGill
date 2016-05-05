#ifndef _INCLUDE_SFS_API_H_
#define _INCLUDE_SFS_API_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "disk_emu.h"
#define MAXFILENAME 20

#define NUM_DATA_POINTERS_IN_INDIRECT 512 //----->2048/4

typedef struct {
    uint64_t magic;
    uint64_t block_size;
    uint64_t fs_size;
    uint64_t inode_table_len;
    uint64_t root_dir_inode;
    uint64_t length_free_block_list;
    uint64_t number_free_blocks;
} superblock_t;

typedef struct {
    unsigned int mode;
    unsigned int taken;
    unsigned int link_cnt;

   
    unsigned int uid;
    unsigned int gid;
    unsigned int size;
    unsigned int data_ptrs[12];
    unsigned int indirect[NUM_DATA_POINTERS_IN_INDIRECT];
    // TODO indirect pointersssssss
} inode_t;

typedef struct{
 
  char *names; //names with inode index.
  int mode;
 // uint64_t rwptr[120];
} directory_table;
/*
 * inode    which inode this entry describes
 * rwptr    where in the file to start
 */
typedef struct {
   
    uint64_t inode;
    uint64_t rwptr;
} file_descriptor;

typedef struct 
{
    uint64_t num;
}free_block_list;
void mksfs(int fresh);
int sfs_getnextfilename(char *fname);
int sfs_getfilesize(const char* path);
int sfs_fopen(char *name);
int sfs_fclose(int fileID);
int sfs_fread(int fileID, char *buf, int length);
int sfs_fwrite(int fileID, const char *buf, int length);
int sfs_fseek(int fileID, int loc);
int sfs_remove(char *file);

#endif //_INCLUDE_SFS_API_H_
