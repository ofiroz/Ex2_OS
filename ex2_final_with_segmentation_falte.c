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
#include <signal.h>
#include <fcntl.h>

#define MAX 510
//---------------------FUNCTION & global-----------------------------------------------------------------------------------------------
	
	int which_kind_of_arrow = 0; // 1 = '>'		2 = '>>'		3 = '<'		4 = '2>'
	 
	int fd; // arrows dup
	 
	int flag = 0;//keeps track if we've came from arrows
	
	int ampersand = 0;//flag that indicate if there is '&'
	
    int checkpipe(char **commands);
	
	char *filename_for_2arrow;
	
	void exe_pipe(char **commands, int counter);
	
	void sig_handler(int signo);
	
	int check_for_arrow(char **commands);
	
	void redirect_right_override(char **commands, int counter);
	
	char* ampersand_checker(char input[]);
	
	void freeMat(char** ary);
	
	void ma();
//----------------------------------------------------------------------------------------------------------------------------
int main()
{
	while(1)
	{	
		signal(SIGINT, sig_handler);//^C
		signal(SIGCHLD, sig_handler);//&
		ma();
	}
    return 0;
}	 

void ma()//the "main" function 
{	
    struct passwd *p;
	uid_t id = 0;
    p = getpwuid(id);
	
	pid_t non_cd_child_pid;//cd
	
	pid_t arrow_child_pid;//arrows
	
    char userInput[MAX];    
    char userInput2[MAX];
    char temp[MAX];
	
    char *ptr,*ptr2; 
	
    char buf[BUFSIZ];
    int stat_loc;
	
    char **commandArray;	
    
	int count = 0, i = 0 ,numOfCommands = 0,lengthOfCommands = 0;
    int cmd = 1;
	 
     printf("%s@%s>", p->pw_name,getcwd(buf,sizeof(buf)));//prompt line
	 fgets(temp,MAX,stdin);//scan cmd
      	 
    while (strcmp(temp,"done\n")) //loop till "done"
    {			
		if(flag == 1)//in case when we CONTINUE from arrows
        {  close(fd);
            printf("%s@%s>", p->pw_name,getcwd(buf,sizeof(buf)));//prompt line
	        fgets(temp,MAX,stdin);//scan cmd
			flag = 0;
        }			
				
        if(!strcmp("\n",temp))//in case when ENTER is entered as the cmd
        {  
            printf("%s@%s>", p->pw_name,getcwd(buf,sizeof(buf)));//prompt line
	        fgets(temp,MAX,stdin);//scan cmd
            continue;
        }	
char temp1[MAX];
  strcpy(temp1,ampersand_checker(temp)); 

		 strcpy(userInput,temp1); //cloning the users input for later use
	     strcpy(userInput2,temp1);

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
	   ptr2 = strtok(userInput2,"\" \n");
       i=0;	
      while(i < count)
	  {
        commandArray[i] = (char*)malloc((strlen(ptr2))*sizeof(char)); //each cmd to its own cell in mat
		commandArray[i]=ptr2;
		ptr2 = strtok(NULL,"\" \n");
		i++;
	  }
      commandArray[i]=NULL;//last cell
	  
	  if(strcmp(commandArray[0],"cd"))//if cmd != "cd"						
      {	
		int checkArrow = check_for_arrow(commandArray);
		if(checkArrow > 0)
		{
		//arrow case first (contains pipe case)
				//the son would go to file and exit, the father continue
				arrow_child_pid = fork();
				if(arrow_child_pid < 0)//check if failed
				{printf("ERR\n");	exit(1);}
				
				if( arrow_child_pid == 0)
				{
					if( which_kind_of_arrow == 1 )// ">"
					{
						flag = 1;
						redirect_right_override(commandArray, check_for_arrow(commandArray));	
						which_kind_of_arrow = 0;			
						exit(0);
					}
					else if( which_kind_of_arrow == 2 )//">>"
					{
						flag = 1;
						redirect_right_override(commandArray, check_for_arrow(commandArray));	
						which_kind_of_arrow = 0;
						exit(0);
					}
					else if( which_kind_of_arrow == 4 )//"2>"
					{
						flag = 1;
						redirect_right_override(commandArray, check_for_arrow(commandArray));
						which_kind_of_arrow = 0;
						exit(0);
					}
				}
			wait(NULL); //parent waits untill son finish in file
		}//DONE ARROWS CASE

		//IF WE'VE GOT A PIPE (without arrows)
		else if(checkpipe(commandArray) > 0)//if there is pipe ("|")
		{
			exe_pipe(commandArray, count);//take care of pipe cmd
		}//DONE PIPE CASE
	
		else//if(checkpipe(commandArray) == -1) //non arrow non pipe non "cd" cmd---> already checked for "&"
		{		
			non_cd_child_pid = fork();
			if(non_cd_child_pid < 0)//check if failed
				{printf("ERR\n");	exit(1);}
				
			if(non_cd_child_pid == 0)
			{
				execvp(commandArray[0], commandArray);//commance the command given-> if failed will print
				printf("ERR\n");
				exit(1); 
			}
			 else  //if parent and there is "&"--> pause
				{
					signal(SIGCHLD,sig_handler);
					if(ampersand == 1){
					pause();
					}
					else
					ampersand = 0;
				}                      
		 }			 
	  }
	else //if "cd" case
		{	
		if(!strcmp(commandArray[0],"cd"))//change directory						
		{																		
			int x = chdir(commandArray[1]);
			if(x==-1) //check if failed
			printf("ERR\n");	
		}	
	}
	if(non_cd_child_pid>0)//father
	  {
		waitpid(non_cd_child_pid, &stat_loc, WUNTRACED);//wait till son finish
	  }
	  
      if(stat_loc == 1)//check if son finished
        {
             for(int i = 0; i < count; i++ )
                free(commandArray[i]);
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
///--------------------------------------------------------------
	//there is no way that | will come with " ", so strtok must saparate it as a word by himself
	int checkpipe(char **commands)//commands = commandArray
	{
		int i = 0;
		while(commands[i])//commands[i]!=null 
		{
			if(!(strcmp(commands[i],"|")))//if there is '|'
			{
				return i; //return the place where '|' is
			}
			i++;				
		}
		return -1;//if there's no pipe retrun -1
	}	
//---------------EXIT IMIDIATE WHEN "^C"-----------------------------------------------------------------------------------------
	void sig_handler(int signo)
	{
		signal(SIGINT, sig_handler);
		int sts; 
		if(signo == SIGINT)
			kill(signo, SIGHUP);

		if(signo == SIGCHLD)
		{
			while(waitpid(-1, &sts, WNOHANG) > 0 );//free all the sons
			}
	}	
//-----------------------------------------------------------------------------------------------------------------------------	

//-----------------------EXE PIPE----------------------------------------------------------------------------------------------
	void exe_pipe(char **commands, int counter)
	{
		//split the commandArray to 2 arrays in order to activate execvp 
				int pipe_fd[2];
				 pid_t left_child_pid;
			     pid_t right_child_pid;
				char **first_half_of_commandArray;
				char **second_half_of_commandArray;	
				
				first_half_of_commandArray = (char**)malloc((checkpipe(commands))*sizeof(char*));
				if(!first_half_of_commandArray)//exit if  failed
				{printf("ERR");   exit(1);}
				
				second_half_of_commandArray = (char**)malloc((counter - checkpipe(commands))*sizeof(char*));
				if(!second_half_of_commandArray)//exit if  failed
				{printf("ERR");   exit(1);}			
				
				int i1;
				for(i1 = 0; i1 < checkpipe(commands); i1++)
				{
					first_half_of_commandArray[i1]= (char*)malloc(*commands[i1]);
					if(!first_half_of_commandArray[i1])//exit if  failed
					{printf("ERR");   exit(1);}
					first_half_of_commandArray[i1] = commands[i1];				
				}
				first_half_of_commandArray[i1] = NULL;;//last cell = NULL
				
				//run from "|" till the last cell in commandArray
				int i2; int f = 0;//f is 0 till end of second ary
				for(i2 = checkpipe(commands)+1; i2 < counter; i2++)
				{
					second_half_of_commandArray[f] = (char*)malloc(*commands[i2]);
					if(!second_half_of_commandArray[f])//exit if  failed
					{
						printf("ERR");   
						exit(1);
					}
					second_half_of_commandArray[f] = commands[i2];
				 f++;
				}
				second_half_of_commandArray[i2] = NULL;//last cell = NULL	
				
		pipe(pipe_fd);
				
		left_child_pid = fork();//create son only when cmd != "cd"
		if(left_child_pid < 0)//check if failed
		{printf("ERR\n");	exit(1);}
				
			if(left_child_pid == 0)				
			{
					//replace standard input with input part of pipe
					dup2(pipe_fd[1], 1);//STDOUT_FILENO=1
					close(pipe_fd[0]);				
					// execute left cmd
					int check1 = execvp(first_half_of_commandArray[0], first_half_of_commandArray);	
					if( check1 < 0)
					{
						printf("%s: command not found\n", first_half_of_commandArray[0]);
						freeMat(commands);	free(first_half_of_commandArray); free(second_half_of_commandArray);//Release memory allocation
						exit(1);
					}					
			}				
			right_child_pid = fork();
			if(right_child_pid < 0)//check if failed
			{printf("ERR\n");	exit(1);}						
					
				if(right_child_pid == 0)
				{
					// replace standard output with output part of pipe
					dup2(pipe_fd[0], 0);//STDIN_FILENO = 0
					close(pipe_fd[1]);
					// execute right cmd
					int check2 = execvp(second_half_of_commandArray[0], second_half_of_commandArray);
					if(check2 < 0)
					{
						printf("%s: command not found\n", second_half_of_commandArray[0]);
						freeMat(commands);	free(first_half_of_commandArray); free(second_half_of_commandArray);//Release memory allocation
						exit(1);
					}			
				}
		int w;//wait by status
		close(pipe_fd[0]);
		close(pipe_fd[1]);
		waitpid(left_child_pid,&w,0);
		waitpid(right_child_pid,&w,0);			
}
//-----------------------------------------------------------------------------------------------------------------------------


//----------------------RIDIRECTION---------------------------------------------------------------------------------------------
		
	//-------IN CASE OF ARROWS----------------------------------------------|
	
		//check where are (and if there is..) the arrows
		// 1 = '>'		2 = '>>'		3 = '<'		-1 = 'non'
		//DONT TAKE CARE OF 4 = '2>'
	int check_for_arrow(char **commands)//commands = commandArray
	{
		int i = 0;
		while(commands[i])//commands[i]!=null 
		{
			if(!(strcmp(commands[i],">")))//if there is '>'
			{
				which_kind_of_arrow = 1;
				return i; //return the place where '>' is
			}
			
			else if(!(strcmp(commands[i],">>")))//if there is '>>'
			{
				which_kind_of_arrow = 2;
				return i; //return the place where '>>' is
			}
			
			else if(!(strcmp(commands[i],"<")))//if there is '<'
			{
				which_kind_of_arrow = 3;
				return i; //return the place where '<' is
			}			
			i++;	
		}
		return -1;//if there's no ">" retrun -1
	}
	
	//write left cmd to right file (works for both > & >>	**TAKE CARE OF PIPE + ARROW**
	void redirect_right_override(char **commands, int counter)
	{
		pid_t child_pid;
		int len = check_for_arrow(commands);
		
		char **left_cmd = (char**)malloc((check_for_arrow(commands)+1)*sizeof(char*));//for the cmd left too the arrow+null
		//** NOTE#1: len of the cmd left too the arrow is "check_for_arrow(commands)"
		//** NOTE#2: the filename is in the "check_for_arrow(commands)+1" cell! && "check_for_arrow(commands)+2" is NULL
		int k=0;
		while(k < check_for_arrow(commands))//run on left to the arrow
		{	
			left_cmd[k] = commands[k];	
			k++;
		}
		left_cmd[k] = NULL;//last cell = null
		
		char filename[MAX];
		strcpy(filename, commands[len+1]);	
		
		//THE CASES--------	
		if(which_kind_of_arrow == 1)// ">"
			fd = open(filename,O_WRONLY |O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
		else if(which_kind_of_arrow == 2)// ">>"
			fd = open(filename,O_WRONLY |O_CREAT | O_APPEND , S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
		else if(which_kind_of_arrow == 4)// "2>"
		{
			int a1=0;
			while(commands[a1])//!=NULL
			{
				a1++;
			}
			//commands[a1-1]//2>filename			
			filename_for_2arrow = strtok(commands[a1-1],"2>");
			fd = open(filename_for_2arrow , O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU );
			if (fd  == -1) { perror("ERR"); exit(1); } 
                 dup2(fd, 2);
		}		
		if(fd==-1)
		{
			fprintf(stderr , "ERROR on file output");
			exit(1);
		}
		
		int value = dup2(fd,STDOUT_FILENO);
		if(value == -1)
		{
			fprintf(stdout,"dup2 failed\n");
			exit(1);
		}

		child_pid = fork();
		if(child_pid < 0)//check if failed
				{printf("ERR\n");	exit(1);}	
		
		if(child_pid == 0)
		{
			if(checkpipe(left_cmd) > 0) //if there is pipe BEFORE THE ARROW (!)
			{
				exe_pipe(left_cmd, counter);
			}
			else{
				execvp(left_cmd[0], left_cmd);//commance the command given-> if failed will print
					printf("ERR\n");
					exit(1);
				}
		}		
		wait(NULL);
		//freeMat(left_cmd);
	}
//----------------------------------------------------------------------------------------------------------------------------

//-------AMPERSAND-------------------------------------------------------------------------------------------------------------

	char* ampersand_checker(char input[])//rais the "ptr_ampersand_search" flag if there is "&" and retrun the string without it
	{																				
		char *ptr_ampersand_search;	
		int be=0; int af=0;
		int i=0; int j=0;
		while(input[i])//!=NULL
		{
			be++; i++;
		}
		ptr_ampersand_search = strtok(input,"&\n");
		
		while(ptr_ampersand_search[j])//!=NULL
		{
			af++; j++;
		}
		if(be != af+1)//if gotten shorter
			ampersand = 1;
		return ptr_ampersand_search;
	}
//-----------------------------------------------------------------------------------------------------------------------------

//-----------------------free mat----------------------
//free all memory allocation
	void freeMat(char** ary)
	{
		int j = 0;
		while(ary[j] != NULL)
		{
			free(ary[j]);
			ary[j] = NULL;
			j++;
		}	
		free(ary);
	}





