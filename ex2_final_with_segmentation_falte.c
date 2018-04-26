/*
* Autor: Ozeri Ofir
* Sid: ofiroz
* Id: 308286566
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <pwd.h>
#include <sys/types.h>

#define MAX 510
//#define EXIT_FAILURE exit(1) //warning

void ma() 
{
    struct passwd *p;
	uid_t id = 0;
    p = getpwuid(id);

    pid_t child_pid;
	
    char userInput[MAX];    
    char userInput2[MAX];
    char userInput3[MAX];
    char temp[MAX];
	
    char *ptr,*ptr2, *ptr3;  
    char buf[BUFSIZ];
    int stat_loc;
	
    char **commandArray; char **commandArray_pipe;
    int count = 0, i = 0 ,numOfCommands = 0,lengthOfCommands = 0;
    int cmd = 1;
	int cnt_pipe = 0;
	
	int pipe_fd[2];
     
     printf("%s@%s>", p->pw_name,getcwd(buf,sizeof(buf)));//prompt line
	 fgets(temp,MAX,stdin);//scan cmd
     
    while (strcmp(temp,"done\n")) //loop till "done"
    {
         strcpy(userInput,temp); //cloning the users input for later use
	     strcpy(userInput2,temp);
	     strcpy(userInput3,temp);		
		
		commandArray_pipe = (char**)malloc((cnt_pipe+1)*sizeof(char*));//ary in the cmd size
	        if(!commandArray_pipe)//exit if  failed
            {
                printf("ERR");
                exit(1);
            }
		
	   commandArray = (char**)malloc((count+1)*sizeof(char*));//ary in the cmd size
	        if(!commandArray)//exit if  failed
            {
                printf("ERR");
                exit(1);
            }
			
			
		 ptr = strtok(userInput3,"|"); //checks if there is any "|"
	     while(ptr3)//ptr!=null
	     {
		    cnt_pipe++;
		    ptr3 = strtok(NULL,"|");
	     }
		   
		 
         ptr = strtok(userInput,"\" \n");//WORD COUNTER
         count=0;
	     while(ptr)//ptr!=null
	     {
		    count++;
		    ptr = strtok(NULL,"\" \n");//igmore ' " '& '\n'
	     }
		 
	  ptr3 = strtok(userInput3,"|");//if pipe 
      int q = 0;	
      while(i < cnt_pipe)
	  {
        commandArray_pipe[q]= (char*)malloc((strlen(ptr3))*sizeof(char));
		commandArray_pipe[q]=ptr3;
		ptr3 = strtok(NULL,"|");
		q++;
	  }
      commandArray_pipe[q]=NULL;
		
		
	   ptr2 = strtok(userInput2,"\" \n");//commands -> to array
       i=0;	
      while(i<count)
	  {
        commandArray[i]= (char*)malloc((strlen(ptr2))*sizeof(char)); //each cmd to its own cell in mat
		commandArray[i]=ptr2;
		ptr2 = strtok(NULL,"\" \n");
		i++;
	  }
      commandArray[i]=NULL;
	
	  if(strcmp(commandArray[0],"cd"))//if cmd != "cd"						
      {
		child_pid = fork();//create son only when cmd != "cd"
		if(child_pid < 0)//check if failed
		{
			printf("ERR\n");
			exit(1);}
		else if (child_pid == 0)//if son
		{   
			if(cnt_pipe > 1)//if there is pipe ("|")
			{
				pipe(pipe_fd);
				
				child_pid = fork();
				if(child_pid < 0)//check if failed
				{
					printf("ERR\n");
					exit(1);}

				if(child_pid == 0)//if sons son
				{
					// replace standard input with input part of pipe
					dup2(pipe_fd[0], 0);
					// close unused hald of pipe
					close(pipe_fd[1]);
					// execute right cmd
					execvp(commandArray_pipe[0], commandArray_pipe);
				}
				
				else{//if is son
						if(child_pid > 0)
						{
						// replace standard output with output part of pipe
						dup2(pipe_fd[1], 1);
						// close unused unput half of pipe
						close(pipe_fd[0]);
						// execute left cmd
						execvp(commandArray_pipe[1], commandArray_pipe);
						}
				}//end of "if pipe"	
					
			}	
				else if(cnt_pipe < 2)		
				{
					execvp(commandArray[0], commandArray);//commance the command given-> if failed will print
					printf("ERR\n");
					exit(1); 
					cmd = 0;
				}
			} 
	  }
		
		else //if papa
			{	
			if(!strcmp(commandArray[0],"cd"))//change directory						
            {																		
              int x = chdir(commandArray[1]);
              if(x==-1) //check if failed
                printf("ERR\n");
				}	
			}
		
	  if(child_pid>0)//father
	  {
		waitpid(child_pid, &stat_loc, WUNTRACED);//wait till son finish
	  }
	  
      if(stat_loc == 1)//check if son finished
        {
             for(int i = 0; i < count; i++ )
                free(commandArray[i]);
             
			 for(int i = 0; i < cnt_pipe; i++ )
                free(commandArray_pipe[i]);
			 free(commandArray);
			 free(commandArray_pipe);
        }
    if(cmd)//count commands
    {
		if(strcmp(commandArray[0],"cd"))
		{
        numOfCommands++;
        lengthOfCommands+= strlen(commandArray[0]);
		}
    }
      printf("%s@%s>", p->pw_name,getcwd(buf,sizeof(buf)));//prompt line
	  fgets(temp,MAX,stdin);//inloop input    
    }
	
    printf("Num of cmd: %d\nCmd length: %d\nBye !\n",numOfCommands,lengthOfCommands);//when "done" typed
   
    exit(0);
}

int main()
{
	ma();
	
    return 0;
}


