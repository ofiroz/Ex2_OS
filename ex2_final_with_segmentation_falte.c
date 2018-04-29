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


//---------------------FUNCTION & global-----------------------------------------------------------------------------------------------
int temp_global_var = 0;
	 
	int if_there_is_pipe_in_wich_cell_is_it = 0;
	 
    int checkpipe(char **commands, int num_of_cmd);
	
	void ma();
//----------------------------------------------------------------------------------------------------------------------------
int main()
{
	ma();
	
    return 0;
}	 


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
	
    char **commandArray;
	char **first_half_of_commandArray;
	char **second_half_of_commandArray;	
    
	int count = 0, i = 0 ,numOfCommands = 0,lengthOfCommands = 0;
    int cmd = 1;

	int pipe_fd[2];
	 
     printf("%s@%s>", p->pw_name,getcwd(buf,sizeof(buf)));//prompt line
	 fgets(temp,MAX,stdin);//scan cmd
     
temp_global_var++;
	 
	 
    while (strcmp(temp,"done\n")) //loop till "done"
    {
			
         strcpy(userInput,temp); //cloning the users input for later use
	     strcpy(userInput2,temp);
	   
//---------------------CMD ARRAY---------------------------------------------------------------------------------------------
		 ptr = strtok(userInput,"\" \n");//WORD COUNTER
         count=0;
	     while(ptr)//ptr!=null
	     {
		    count++;
		    ptr = strtok(NULL,"\" \n");//igmore ' " '& '\n'
	     }
		commandArray = (char**)malloc((count+1)*sizeof(char*));//ary in the cmd size
	        if(!commandArray)//exit if  failed
            {printf("ERR");   exit(1);}
			
//---------------------------------------------------------------------------------------------------------------------------		
	
	   ptr2 = strtok(userInput2,"\" \n");//commands -> to array
       i=0;	
      while(i<count)
	  {
        commandArray[i]= (char*)malloc((strlen(ptr2))*sizeof(char)); //each cmd to its own cell in mat
		commandArray[i]=ptr2;
		ptr2 = strtok(NULL,"\" \n");
		i++;
	  }
      commandArray[i]=NULL;//last cell

//------------------IS THERE A PIPE--------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------
	  if(strcmp(commandArray[0],"cd"))//if cmd != "cd"						
      {
		child_pid = fork();//create son only when cmd != "cd"
		if(child_pid < 0)//check if failed
		{printf("ERR\n");	exit(1);}
		
		else if (child_pid == 0)//if son
		{   
//------------------------IF WE'VE GOT A PIPE-------------------------------------------------------------
			if(checkpipe(commandArray, count) == 0)//if there is pipe ("|")
			{
printf("\nIN PIPE CMD\n");
				//***now  i have to split the commandArray to 2 arrays in order to activate execvp 	
		//----the 2 halfs of the cmd ary--> left/right to the pipe-------------------|
				first_half_of_commandArray = (char**)malloc((if_there_is_pipe_in_wich_cell_is_it)*sizeof(char*));
				if(!first_half_of_commandArray)//exit if  failed
				{printf("ERR");   exit(1);}
				
				second_half_of_commandArray = (char**)malloc((count - if_there_is_pipe_in_wich_cell_is_it)*sizeof(char*));
				if(!second_half_of_commandArray)//exit if  failed
				{printf("ERR");   exit(1);}
				
				{
				int i1;
				for(i1 = 0; i1 < if_there_is_pipe_in_wich_cell_is_it; i1++)
				{
					first_half_of_commandArray[i1]= (char*)malloc(*commandArray[i1]);
					if(!first_half_of_commandArray[i1])//exit if  failed
					{printf("ERR");   exit(1);}
					first_half_of_commandArray[i1] = commandArray[i1];
				}
				first_half_of_commandArray[i1];//last cell = NULL
				
				//run from "|" till the last cell in commandArray
				int i2;
				for(i2 = if_there_is_pipe_in_wich_cell_is_it+1; i2 < count; i2++)
				{
					second_half_of_commandArray[i2]= (char*)malloc(*commandArray[i2]);
					if(!second_half_of_commandArray[i2])//exit if  failed
					{printf("ERR");   exit(1);}
					second_half_of_commandArray[i2] = commandArray[i2];
				}
				second_half_of_commandArray[i2];//last cell = NULL
		//----------------------------------------------------------------|
				pipe(pipe_fd);
				child_pid = fork();
				if(child_pid < 0)//check if failed
				{printf("ERR\n");	exit(1);}

				if(child_pid == 0)//if sons son
				{
					// replace standard input with input part of pipe
					dup2(pipe_fd[0], 0);
					// close unused hald of pipe
					close(pipe_fd[1]);
					// execute right cmd
					execvp(first_half_of_commandArray[0], first_half_of_commandArray);
				}
				
				else{//if is son
						if(child_pid > 0)
						{
						// replace standard output with output part of pipe
						dup2(pipe_fd[1], 1);
						// close unused unput half of pipe
						close(pipe_fd[0]);
						// execute left cmd
						execvp(second_half_of_commandArray[1], second_half_of_commandArray);
						}
				}//end of "if pipe"	
			}				
		}
//--------------------------------------------------------------------------------------------------------			
				else if(checkpipe(commandArray, count) == -1)	
				{
printf("in non cd non pipe cmd");
					execvp(commandArray[0], commandArray);//commance the command given-> if failed will print
					printf("ERR\n");
					exit(1); 
					//cmd = 0;
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
            /* 
			 for(int i = 0; i < cnt_pipe; i++ ) //FREE ALL THE ARRAYS!!! DONT FORGET second_half_of_commandArray
                free(commandArray_pipe[i]);
			 free(commandArray);
			 free(commandArray_pipe);
			*/
		}
    if(cmd)//count commands
    {
		if(strcmp(commandArray[0],"cd"))
		{
        numOfCommands++;
        lengthOfCommands+= strlen(commandArray[0]);
		}
    }
	
printf("\n** %d **\n", if_there_is_pipe_in_wich_cell_is_it);	
      printf("%s@%s>", p->pw_name,getcwd(buf,sizeof(buf)));//prompt line
	  fgets(temp,MAX,stdin);//inloop input    
    
	}
	
    printf("Num of cmd: %d\nCmd length: %d\nBye !\n",numOfCommands,lengthOfCommands);//when "done" typed
    exit(0);
}
///--------------------------------------------------------------
	//there is no way that | will come with " ", so strtok must saparate it as a word by himself
	int checkpipe(char **commands, int num_of_cmd)//commands = commandArray // cmd_size = count
	{
		for(int i=0;i<num_of_cmd;i++)
		{ 
			if(!(strcmp(commands[i],"|")))//if there is '|'
			{
				if_there_is_pipe_in_wich_cell_is_it = i;//thats how we'll know where is the "|"
//printf("\n returning '0' in checkpipe");
				return 0;
			}	
printf("\ntemp_global_var: %d\n",temp_global_var);
			if_there_is_pipe_in_wich_cell_is_it++;//that is how we know in wich cell is |
printf("\n** %d **\n", if_there_is_pipe_in_wich_cell_is_it);
		}	
//printf("\n** %d **\n", if_there_is_pipe_in_wich_cell_is_it);		
		return -1;
	}	
///--------------------------------------------------------------







