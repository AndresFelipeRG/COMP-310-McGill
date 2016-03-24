#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define MAX_JOBS 20//maximum of jobs.
#define FG 1 //job in foreground.
#define BG 2//job in background.
#define ST 3//job stopped.
#define NOSTATE 4//Undefined state.
#define MAX_ARGS 20
#define MAX_LENGTH 100
#define READ_END 0
#define WRITE_END 1
typedef struct _Job{

	int  num_args;
	
	int job_id2;
	int pid2;
	
	char ** c_args;
	int bg;
	
}Job;

typedef struct _Command {
	int  n;
	int num_args;
	int  bad_command;
	int bg;
	char **c_args;
	struct _Command * next;
}Command;
int *counter = NULL;
int *counter2 = NULL;
Command * head = NULL;
Command * tail = NULL;

void jobs2();
int next_job_id = 1;

Job array_jobs[1000];

int addjob2(int num_args,pid_t  pid, int position, char ** args, int counter3);

int deljob2(int pid);
void push(char ** args,int num_args, int bg){
	//sleep(0.5);
	int counter2 = 0;
	if(head == NULL){
	    head = (Command*)malloc(sizeof(Command));
	    tail = head;
	    counter2 = 0;
	    tail->n = counter2;
	    tail->bg = bg;
	    head->bg = bg;
	    head->n = counter2;
        tail->num_args = num_args;
	    head->num_args = num_args;
	    head->next = (Command*)malloc(sizeof(Command));

           // printf("ADDING HEAD 1ST COMMAND\n");
	   // return;
	}
	else{
	    tail->next = (Command*)malloc(sizeof(Command));
	    counter2 = tail->n;
	    tail = tail->next;
	    tail->n = counter2 +1;
	}
	
	tail->c_args = NULL;
	tail->c_args =(char**)(malloc(sizeof(args)));

	int i = 0;
	for(i = 0; i < num_args; i++){
		 char * my_copy; my_copy = malloc(100 * strlen(args[i])); strcpy(my_copy,args[i]); 
	     tail->c_args[i] = my_copy;
	}
//	tail->c_args[i] ='\0';
	(tail->bad_command) = 0;
	tail->bg = bg;
	tail->num_args = num_args; 
	//printf("Number of commands stored so far:  %d\n",(tail->n));
}


int getcmd(char *prompt, char *args[], int *background)
{
	
    //sleep(0.5);
   
    int length, i = 0;
    char *token, *loc;
    char *line;
    size_t linecap = 0;
    printf("\n");
    printf("%s", prompt);
    length = getline(&line, &linecap, stdin);//need to free getline
    
  



    // Check if background is specified..
    if ((loc = index(line, '&')) != NULL) {
        *background = 1;
        *loc = ' ';
    } else
        *background = 0;

    while ((token = strsep(&line, " \t\n")) != NULL) {
	int j = 0;
        for (j = 0; j < strlen(token); j++){
            if (token[j] <= 32){
            	
                token[j] = '\0';
            }}
        if (strlen(token) > 0){
                
		args[i++] = token;
		
	}
    }
   

    
	args[i++] = '\0';
	
	
	if(args[0] == NULL){return -1;}
    push(args,i-1, *background);
    //free(line);

    return i;
}
int main(){

 	char *args[20];
 	//char *args2[20];
    int bg;
    //int bg2;
    int * number;
    
 
    
    int status;
    int i = 0;
    int k = 0;
    char buff[10];
    counter= &i;
    counter2 = &k;

 while(1)
 {
 	char write_msg[20] = "FAIL";
 	char write_msg2[20] = "GOOD";
 	char * read_msg = (char*) malloc(100*sizeof(char));
 	int fd[2];
 	close(3);
 	close(4);
    pid_t pid;
	
	
	int cnt = getcmd("\n>>  ", args, &bg);
	
	if(cnt == -1){
	head = NULL;
	printf("Exiting\n");
	if(head!=NULL){
	free(head);}
		
	exit(0);
		
	

	}
	if(cnt == -2){
	continue;

	}
	if(strcmp("EOF",args[0])==0||strcmp(args[0],"exit")==0){
	printf("Exiting\n");
	if(head!=NULL){
	free(head);}
		
	exit(0);}
    int i = 0;
    if(atoi(args[0])  > 0 && atoi(args[0]) >= tail->n){
			printf("\nno command found in history\nThis is not a valid number in history. Not enough commands stored so far.\n");
			tail->bad_command = 1;
			continue;
		}

	if(atoi(args[0]) < 0){
			printf("\nno command found in history\n\nThis is not a valid number in history. Enter a positive number.\n");
			
			tail->bad_command = 1;
			continue;
		}
    if(atoi(args[0]) <= tail->n  && atoi(args[0])> 0 ){
    	if(head == NULL){
    		printf("\nno command found in history\n\nThere are no commands in history.\n");
			
			tail->bad_command = 1;
			continue;
    	}

		if(args[1]!=NULL){
			
			tail->bad_command = 1;
			printf("\nno command found in history\n\nToo many arguments entered.\n");
			continue;

		}	
        int target = atoi(args[0]);
        if(target < 0){
			printf("\nno command found in history\n\nThis is not a valid number in history. Select a command among the last stored commands.\n");
			
			tail->bad_command = 1;
			continue;
		}
		if(target <=(tail->n)-11 && (tail->n)>9){

			printf("\nno command found in history\n\nThis is not a valid number in history. Select a command among the last 10 stored commands.\n");
			
			tail->bad_command = 1;
			continue;
		}
		
		if((target > 0)){

			int counter = 0;
			Command * iterate = (Command*)malloc(sizeof(Command));
			iterate = head;
			//printf("\nTHe target %d\n", target);
			while(counter < target-1){
				iterate  = iterate->next;
				counter++;
			}
			
			int i = 0;
			if(iterate!= NULL){
				if(iterate->bg == 1){
					bg = 1;
					printf("\nThis command is going to run in the background.\n");
				}

			}
			tail->c_args = NULL;
			tail->c_args =(char**)(malloc(sizeof(args)));
			for(i = 0; i < iterate->num_args; i++){
		     char * my_copy; my_copy = malloc(sizeof(char) * strlen(iterate->c_args[i])); strcpy(my_copy,iterate->c_args[i]); 
	    	 tail->c_args[i] = my_copy;
		 tail->bg = bg;
	    	// printf("\narguments %s\n", iterate->c_args[i]);
	    	 fflush(stdout);
			}
			if(strcmp("jobs",tail->c_args[0])==0 ){
			args[0] ="jobs";
			args[1] = '\0';
		}
			if(strcmp("history",tail->c_args[0])==0 ){
			args[0] ="history";
			args[1] = '\0';
		}
		
			tail->num_args = iterate->num_args;
			tail->c_args[i++] ='\0';
		}
		//bg = 1;
		//printf("\nThis command is going to run in the background.\n");
		fflush(stdout);

	}
    
    for (i = 0; i < cnt-1; i++){
        printf("\nArg[%d] = %s", i, args[i]);}
    printf("\n");
    if(pipe(fd)==-1){

    	printf("\nThe pipe creation failed.Try again\n");
  	tail->bad_command = 1;
	continue;
    }

 	pid = fork();
    if(pid <0){
	fprintf(stderr, "Fork Failed");
	tail->bad_command = 1;
	continue;
	}

		
	else if (pid == 0 ){
	close(fd[READ_END]);
  
    if(strcmp(args[0],"cd")==0){exit(0);}
    if(strcmp(args[0],"fg")==0){exit(0);}
    if(strcmp(args[0],"jobs")==0){exit(0);}
    
    if(strcmp(args[0],"history")==0){exit(0);}
   
    if(strcmp(args[0],"pwd")==0 && args[1] == NULL){
	char * pwd = (char*) malloc(MAX_LENGTH*sizeof(char));
	if(built_in_pwd(pwd,MAX_LENGTH)==-1){
		tail->bad_command = 1;
		
		printf("Pwd failed. Try again\n");
		free(pwd);
		write(fd[WRITE_END],write_msg,strlen(write_msg));
		close(fd[WRITE_END]);
		fflush(stdout);
		exit(1);
		
	}
	else{
		printf("Current working directory: %s\n",pwd);
	
	     
		free(pwd);
		fflush(stdout);
		exit(0);
		 
	}
	}
    printf("\n");
    	//Numeric command case
    if(atoi(args[0]) <= tail->n && atoi(args[0])> 0 ){
		if(args[1]!=NULL){
			printf("\nno command found in history\n\nToo many arguments entered.\n");
			write(fd[WRITE_END],write_msg,strlen(write_msg));
			close(fd[WRITE_END]);
			exit(1);

		}	
		printf("\nNumeric command entered %d . \n", atoi(args[0]));

		Command * it = head;
		int target = atoi(args[0]);
		if(target< 0){
			printf("\nno command found in history\n\nThis is not a valid number in history. Select a command among the last 10 stored commands.\n");
			write(fd[WRITE_END],write_msg,strlen(write_msg));
			close(fd[WRITE_END]);
			exit(1);
		}
		if(target <=(tail->n)-11&& (tail->n)>9){

			printf("\nno command found in history\n\nThis is not a valid number in history. Select a command among the last 10 stored commands.\n");
			write(fd[WRITE_END],write_msg,strlen(write_msg));
			close(fd[WRITE_END]);
			exit(1);
		}
		if(target > 0 && target > tail->n){
			printf("\nno command found in history\n\nThis is not a valid number in history. Not enough commands stored so far.\n");
			write(fd[WRITE_END],write_msg,strlen(write_msg));
			close(fd[WRITE_END]);
			exit(1);
		}
		while(it!= NULL && (it->n)<target-1){

			it = it ->next;
		}
		
		//printf("\n%d\n", it->num_args);
		//printf("\n%s\n",it->c_args[0]);
		fflush(stdout);
		if(it->bad_command == 1){
			//close(fd[0]);
			
			//write(fd[1],string,(strlen(string)+1));
			//close(fd[1]);
			printf("\nErroneous command. You are trying to execute a command that failed previously. Try another command from history\n");
			write(fd[WRITE_END],write_msg,strlen(write_msg));
			close(fd[WRITE_END]);
			fflush(stdout);
			exit(1);
		}


		if(it->num_args-3>=0){
	 if(strcmp(it->c_args[(it->num_args)-2],">")==0){
		if(it->c_args[(it->num_args)-1]==NULL){
		printf("\nNo file was provided to  > .\n");
///		tail->bad_command = "true";	
              //  close(fd[0]);		
		//write(fd[1],string,(strlen(string)+1));
		//close(fd[1]);
		write(fd[WRITE_END],write_msg,strlen(write_msg));
		close(fd[WRITE_END]);
		exit(1);
			

			}
			else{

		printf("\nRedirecting command to file: %s\n", it->c_args[(it->num_args)-1]);

//		fflush(stdin);	
		char* file =  it->c_args[(it->num_args)-1];
		int i = 0;
		int max = strlen(it->c_args[(it->num_args)-1]);
		//printf("\nThe length of the file is %d\n", max);
		char* commands[100];
		for(i = 0; i < (it->num_args)-2;i++){
			commands[i] =it->c_args[i];
			//printf("\nARGS: %s\n", commands[i]);
			fflush(stdout);
		}
	
			
		char  token[1] ;
		token[0] = '\0';
		//i++;
		it->c_args[(it->num_args)-1] = file;
		commands[i++] = '\0';

		int f1;
		printf("\nRedirecting command to file: %s\n", file);
		close(1);
		
		 if((f1= open(file, O_RDWR| O_CREAT|O_APPEND|O_TRUNC, 0666))==-1){
			tail->bad_command = 1;
				
//
			perror("\nFailing to open the file.\n"); 
  			exit(1);
		}
		//args[cnt-3] = token;
		execvp(commands[0], commands);
//
		printf("\nOutput redirection failed\n");
		
		
		write(fd[WRITE_END],write_msg,strlen(write_msg));
		close(fd[WRITE_END]);
		fflush(stdout);
		exit(1);

			}

		}
		//end of command redirection.

	}
		Command * it2 = head;
		int target2 = atoi(args[0]);
		if(target2 < 0){
			printf("\nno command found in history\n\nThis is not a valid number in history. Select a command among the last 10 stored commands.\n");
			write(fd[WRITE_END],write_msg,strlen(write_msg));
			close(fd[WRITE_END]);
			exit(1);
		}
		if(target2 <=(tail->n)-11 && (tail->n)>9){

			printf("\nno command found in history\n\nThis is not a valid number in history. Select a command among the last 10 stored commands.\n");
			write(fd[WRITE_END],write_msg,strlen(write_msg));
			close(fd[WRITE_END]);
			exit(1);
		}
		if(target2 > 0 && target2 > tail->n){
			printf("\nno command found in history\n\nThis is not a valid number in history. Not enough commands stored so far.\n");
			write(fd[WRITE_END],write_msg,strlen(write_msg));
			close(fd[WRITE_END]);
			exit(1);
		}

		while(it2!= NULL && (it2->n)<target2-1){

			it2 = it2 ->next;
		}
		
		//printf("\n%d\n", it2->num_args);
		//printf("\n%s\n",it2->c_args[0]);

		fflush(stdout);
		if(it2->bad_command == 1){
			//close(fd[0]);
			
			//write(fd[1],string,(strlen(string)+1));
			//close(fd[1]);
			printf("\nErroneous command. You are trying to execute a command that failed previously. Try another command from history\n");
			write(fd[WRITE_END],write_msg,strlen(write_msg));
			close(fd[WRITE_END]);
			fflush(stdout);
			exit(1);
		}

		execvp(it2->c_args[0],it2->c_args);
		write(fd[WRITE_END],write_msg,strlen(write_msg));
	
 		printf("\nExecuting the required command from history failed.\n");
	
  
    	close(fd[WRITE_END]);
	        
		//printf("\nExecuting the required command from history failed.\n");
		fflush(stdout);
		exit(0);
	}
	
	
	 /*if(strcmp("jobs",args[0])==0 && args[1] == NULL){
	printf("\nCalling jobs..\n");
	jobs();
	printf("\nFinishing jobs execution.\n");
	
	exit(0);
	}*/
		if(cnt-3>0){
	 if(strcmp(args[cnt-3],">")==0){
		if(args[cnt-2]==NULL){
		printf("\nNo file was provided to  > .\n");
		write(fd[WRITE_END],write_msg,strlen(write_msg));
	
 
	
  
    	close(fd[WRITE_END]);
		
		exit(1);
			

			}
			else{

		printf("\nRedirecting command to file: %s\n", args[cnt-2]);

//		fflush(stdin);	
		char* file =  args[cnt-2];
		int i = 0;
		int max = strlen(args[cnt-2]);
		
		char* commands[100];
		for(i = 0; i < cnt-3;i++){
			commands[i] = args[i];
			
			fflush(stdout);
		}
	
			
		char  token[1] ;
		token[0] = '\0';
		//i++;
		//args[cnt-1] = file;
		commands[i++] = '\0';

	
		printf("\nRedirecting command to file: %s\n", file);
		close(1);
		int fi;
		 if((fi= open(file, O_RDWR| O_CREAT|O_APPEND|O_TRUNC, 0666))==-1){
			tail->bad_command = 1;
				
//
			perror("\nFailing to open the file.\n"); 
			write(fd[WRITE_END],write_msg,strlen(write_msg));
			close(fd[WRITE_END]);
  			exit(1);
		}
		//args[cnt-3] = token;
		execvp(commands[0], commands);
//
		printf("\nOutput redirection failed\n");
		write(fd[WRITE_END],write_msg,strlen(write_msg));
	
 
	
  
    	close(fd[WRITE_END]);
		
		fflush(stdout);
		exit(1);

			}

		}
		//end of command redirection.

	}

	
	 fflush(stdout);
	 int res =  execvp(args[0],args);

	 write(fd[WRITE_END],write_msg,strlen(write_msg));
	
 
	
    printf("\n***User command was not executed succesully in  child process.***\n");
    //printf("\nMESSAGE RECEIVED:%s\n",read_msg);
    //close(fd[WRITE_END]);
    close(fd[WRITE_END]);
    exit(1);
//end of child process
	}
    else{
    		//BEGINNING OF PARENT
    	//fcntl(READ_END, F_SETFL, O_NONBLOCK);
    	
    	
         close(fd[WRITE_END]);
    	 if(strcmp(args[0],"fg")==0){
    	 	if(args[1] == NULL){
    	 		printf("\nYou didnt provide a PID job.\n");
    	 		tail->bad_command = 1;
    	 		fflush(stdout);
    	 	}
    	 	else{
    	 		int counter = 0;
    	 		int close = 0;
    	 	for(counter = 0; counter < 1000; counter++){
			if(array_jobs[counter].pid2 == atoi(args[1])){
				printf("\nPutting PID job %d in the foreground.You will have to wait.\n",atoi(args[1]));
    	 		fflush(stdout);
				waitpid(atoi(args[1]),&status,0);
				if(deljob2(atoi(args[1]))==0){
							printf("\nELiminating job ID: %d from the jobs list\n", pid);
							
							
								}
						else{

						   printf("\nElement job ID: %d couldnt be elimated from the job list.\n", pid);
						   tail->bad_command = 1;
							
							}
				close = 1;
				break;
			}
				
				if(close == 0){
				printf("\nYou didnt provide an exitant PID job.\n");
    	 		fflush(stdout);}
				
				
			
			
    	 	}}
    	 	//continue;
    	 }
    	
    	 if(strcmp("jobs",args[0])==0 ){
    	 	if(args[1]!= NULL){
    	 		printf("\nToo many arguments provided to jobs\n");
    	 		//delete from jobs also from commands..dont store..do it for all your built in commands.
    	 		tail->bad_command = 1;
    	 		continue;
    	 		
    	 	}
    	 	
    	 	else{
	printf("\nCalling jobs..\n");
	int status3;
	printf("\nUpdating the jobs list.\n");
	while((pid = waitpid(-1,&status3,WNOHANG))>0){
		//printf("\nChild %d terminated\n", pid);
		//sleep(0.5);
		
				if(deljob2(pid)==0){
							printf("\nELiminating job ID: %d from the jobs list\n", pid);
							
							
								}
						else{

						   //printf("\nElement job ID: %d couldnt be eliminated from the job list.\n", pid);
							tail->bad_command = 1;
							}
			}
			

		fflush(stdout);
	
	jobs2();}
	printf("\nFinishing jobs execution.\n");
	fflush(stdout);
	continue;	
	
	}
    	if((strcmp("history",args[0]))==0){
	if(args[1] != NULL){
		printf("\nToo many arguments were given to command history. Try again\n");
		fflush(stdout);
		tail->bad_command = 1;
    	 continue;
		//close(fd[0]);
		//write(fd[1],string,(strlen(string)+1));
		//close(fd[1]);
		
		}
	else{
		Command *temp = head;
		int max = tail->n-1;
		int count = 0;
	
		if(max-9>0){
		while(count< max-9){
			
			temp = temp->next;	
			if(temp == NULL){
				printf("\nHistory changed. Try again\n");
				tail->bad_command = 1;
    	 		
				break;
				//tail->bad_command = "true";
				fflush(stdout);
		//	        close(fd[0]);
		//		write(fd[1],string,(strlen(string)+1));
		//		close(fd[1]);
				exit(1);
			}
			count = temp->n;
		}

	}

    printf("\nHistory:\n");
	fflush(stdout);	
	int k = count;		
	while(temp!=NULL && k< tail->n){
		printf("\nCommand number: %d. Num args: %d .Command and args: ",(temp->n)+1,temp->num_args);
	
		int i = 0;
		for(i = 0; i< (temp->num_args);i++){
			if(temp->c_args[i] !=NULL){
			printf("%s ",temp->c_args[i]);
			}
			fflush(stdout);}
			if(temp->bg ==1){
				printf("&\n");
			}
		printf("\n");
		k = k +1;
		temp = temp->next;fflush(stdout);}
	printf("\n");
	//exit(0);
	}    
		//END OF HISTORY
		
	 }
	    if (bg){

        printf("\nBackground enabled..\n");
        
		fflush(stdout); 
		char *myargs[20];
		int i = 0;
		for(i = 0; i < cnt; i++){
			myargs[i] = args[i];
			}
		myargs[i] ='\0';
		
        if(addjob2(cnt,pid,tail->n,myargs,bg)==0){
		printf("\n>>Job added to list of background jobs\n");
		printf("\n");
		fflush(stdout);

		}
  	
	
    	if(strcmp(args[0],"cd")==0){
	if((built_in_cd(args)!=0)){
	//	tail->bad_command = "true";
		printf("\ncd command failed. Try again\n");
		fflush(stdout);
		tail->bad_command = 1;
    	continue;
	}
	else
	{        
		printf("\nChanged current directory.\n");
		fflush(stdout);	
	    tail->bad_command = 1;
    	continue;
	}
	}

		}
else{
	printf("\n\n\n");
	
   

        
		waitpid(pid,&status,0);
		printf("\nBackground not enabled \n");
		//close(fd[WRITE_END]);
		fcntl(fd[READ_END], F_SETFL, O_NONBLOCK);
    	read(fd[READ_END],read_msg,sizeof(read_msg));
    	close(fd[READ_END]);
    	//if(strcmp(read_msg,"FAIL")==0){
    //	printf("\nMESSAGE RECEIVED:%s\n",read_msg);
    	//printf("\nMfd:%d\n",fd[1]);
    	//printf("\nMfd:%d\n",fd[0]);
    	if(strcmp(read_msg,"FAIL")==0){
    		tail->bad_command = 1;
    	}
    	//}
    	if(strcmp(args[0],"cd")==0){
	if((built_in_cd(args)!=0)){
		tail->bad_command = 1;
		printf("cd command failed. Try again\n");
		tail->bad_command = 1;
    	
		fflush(stdout);
		continue;
		
	
	}
	else
	{	
		printf("Changed current directory.\n");
		fflush(stdout);
	
	}
	}
	
	fflush(stdout); 
	
	//end of no background.
	

	
}
fflush(stdout); 
 printf("\n\n");
//END OF PARENT PROCESS
//close(READ_END);
 int k = 0;
// printf("\nEliminating arguments %d\n", cnt);
 args[k] = NULL;
 //fsync(fd[0]);
 //fdatasync(3);
 read_msg = NULL;
 //fdatasync(4);
 //fsync(fd[1]);
}
 printf("\n\n");} 
//    free(head_j);
  //  free(head);
    //free(tail_j);
    //free(tail);
    //return 0;
//free(args);


//end of main
return 0;
}
int addjob2(int num_args,pid_t  pid, int position, char ** args, int counter3){
	position = position % 1000;
	array_jobs[position].num_args = num_args;
	array_jobs[position].bg = counter3;
	array_jobs[position].pid2 = pid;
	array_jobs[position].job_id2 = position;
	array_jobs[position].c_args =(char**)(malloc(sizeof(args)));
	int i = 0;
	for(i = 0; i < num_args-1; i++){
		 char * my_copy; my_copy = malloc(sizeof(char) * strlen(args[i])); strcpy(my_copy,args[i]); 
	 	 array_jobs[position].c_args[i] = my_copy;
	     
	     fflush(stdout);
	}
	array_jobs[position].c_args[i++] ='\0';
	//free(myargs);
	printf("\nAdded job %d. Number of arguments %d. Counter %d Command %s.\n", pid, num_args,position, args[0]);
	fflush(stdout);printf("\n");
	//
	return 0;
}
int deljob2(int pid){
	int counter = 0;
	for(counter = 0; counter < 1000; counter++){
		if(array_jobs[counter].pid2 == pid){
		   array_jobs[counter].pid2 = -100000;
		   array_jobs[counter].c_args = NULL;
		   return 0;
		}
	}
	return -1;
}
int built_in_cd(char**args){
	if(args[1]==NULL){
		printf("Missing directory argument\n");
		fflush(stdout);
		printf("\n");
		return -1;
	}
	else{
		if(chdir(args[1])!= 0){
		printf("No such directory %s\n", args[1]);
		

		printf("\n");
		return -1;
		}
	}
	return 0;
	}
int built_in_pwd(char*buffer, size_t size){
	char* dir = getcwd(buffer,size);
	if(dir == NULL){
		printf("Pwd failed.\n");
		return -1;
	}
	else{
		return 1;	
	}
	return -1;
}
void jobs2(){
	int counter = 0;
	printf("\nList of jobs running in the background:\n");
	for(counter = 0; counter < 1000; counter++){
		if(array_jobs[counter].pid2 > 0){
		   printf("\nJob ID: %d.Commands: \n",array_jobs[counter].pid2);
		   fflush(stdout);
		
		int i = 0;
		for(i = 0; i< (array_jobs[counter].num_args-1);i++){
			if(array_jobs[counter].c_args[i]!=NULL){
			printf("%s ",array_jobs[counter].c_args[i]);
			}
			fflush(stdout);
				}
				  if(array_jobs[counter].bg == 1){printf("&\n");}
			}
	      
		
		 fflush(stdout);

	}

}
