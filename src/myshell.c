/*
 * myshell.c
 *
 *  Created on: Feb 2, 2018
 *	    Author: Matsoukas Vasileios
 *     
 *
 *  	Aristotle University of Thessaloniki,
 *  	Dept. of Electrical & Computer Engineering
 *  	Course: Operating Systems, 7th Semester
 *
 *
 */


#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<unistd.h>

#define BUFSIZE 512
#define BUFSIZE2 128
#define DELIM ";&&"
#define ARGS_DELIM " \t\r\n\a"
#define ARGS_BUFSIZE 64


char* read_line(void);
char* read_batch_line(void);
char** extract_commands(char *);
char** extract_args(char *);
int execute_cmd(char **args);
int total_builtin_cmds(void);
int builtin_cd(char **args);
int builtin_exit(char **args);
char delims[BUFSIZE2];
char *builtin_cmds[]={"cd","quit"};
int (*builtin_func[]) (char **) = { &builtin_cd, &builtin_exit};
FILE *fp;

void main(int argc, char** argv)
{
 char *line;
 char **commands;
 char **args;
 char c;
 int status,sz=0;
 int i=0;
 int f=1;

 if(argc>2)
 {
	 printf("Too many arguments\n");
	 exit(0);
 }
 //If in batch mode. Check for batchfile existence.
 //Calculate batchfile's size.
 if(argc==2)
 {
	fp =fopen(argv[1],"r");
	if(!fp)
	{
		printf("Unable to open file!\n");
		exit(1);
	}
	fseek(fp, 0, SEEK_END);
	sz = ftell(fp);
	rewind(fp);
	if(sz<=1)
	{
		printf("Empty Batch File!\n");
		fclose(fp);
		exit(1);
	}
 }

 printf("Initiallizing MyShell..\n");

//Start infinite loop. Break loop with quit command (Interactive/Batch Mode)
//or by reaching EOF (Batch Mode)
 do {
	 //Interactive Mode when argc==1. Batch Mode when argc==2
	switch(argc)
	{
		case 1:
			if(f)
			{
				printf("Interactive Mode\n\n");
				f=0;
			}
			printf("matsoukas_8743> ");
			line=read_line();
			break;
		case 2:
			if(f)
			{
				printf("Batch Mode\n\n");
				f=0;
			}
			if(ftell(fp)<sz)
			{
				line=read_batch_line();

				if(ftell(fp)>=sz)
				{
					printf("\nReached End Of File.No quit instruction found.Terminating..\n");
					fclose(fp);
					free(line);
					free(args);
					free(commands);
					exit(1);
				}
			}
	}
	commands=extract_commands(line);
	while (commands[i]!=NULL)
	{
		args=extract_args(commands[i]);
		status=execute_cmd(args);
		//Status corresponding to quit command
		if(status==10)
		{
			//Check if quit command is followed by other commands in Batch Mode
			if (argc==2)
			{
				fseek(fp, +1, SEEK_CUR);
				if (ftell(fp)<sz)
				{
					printf("Quit instruction found but it seems that batch file isn't over\n");
					printf("Do you want to continue? y or n \n");
					c=getchar();
					if (c=='y')
					{
						fseek(fp, -1, SEEK_CUR);
						break;
					}
					else if(c=='n')
					{
						printf("Terminating MyShell.. \n");
						free(line);
						free(args);
						free(commands);
			     		exit(10);
					}
				}
				else
				{
					printf("Terminating MyShell.. \n");
					free(line);
					free(args);
					free(commands);
					exit(10);
				}
			}
			else if(argc==1)
			{
				printf("Terminating MyShell.. \n");
				free(line);
				free(args);
				free(commands);
				exit(10);
			}

		}
		//If a command failed to executed and is linked with &&, then the rest of linked commands don't execute.
		if(status!=0 && delims[i]=='&')
		{
			break;
		}

		i++;
	}
	i=0;
	free(args);
	free(commands);
   }while (1);
}

//Read line in interactive mode
char * read_line(void)
{
  int bufsize = BUFSIZE;
  char *linebuffer = malloc(sizeof(char)*bufsize);
  int index = 0;
  int c;

  if (!linebuffer) {
    fprintf(stderr, "Allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1)
  {
    // Read a character
    c = getchar();
    // If we hit EOF, replace it with a null character and return.
    if (c == EOF || c == '\n' )
    {
      linebuffer[index] = '\0';
      return linebuffer;
    }
    else
    {
      linebuffer[index] = c;
    }
    index++;

    // If we have exceeded the buffer, reallocate.
    if (index >= bufsize)
    {
    	bufsize += BUFSIZE;
    	linebuffer = realloc(linebuffer, bufsize);
    	if (!linebuffer)
    	{
    		fprintf(stderr, "Allocation error\n");
    		exit(EXIT_FAILURE);
    	}
    }
  }
}

//Read line in batch line mode.
char *read_batch_line()
{
	int bufsize = BUFSIZE;
	char *linebuffer =malloc(sizeof(char)*bufsize);
	if (!linebuffer)
	{
		fprintf(stderr, "Allocation error\n");
		exit(EXIT_FAILURE);
	}
	fgets(linebuffer,bufsize,fp);
	if(strlen(linebuffer)>1)
	{
		linebuffer[strlen(linebuffer)-1]='\0';  //replace the \n character with \0 for compatibility with interactive mode
	}
	return linebuffer;
}

//Tokenize line into commands. Identify delimiters between commands (; or &&)
//Default command buffer: 128 . Reallocate for more.
//Parse commands using ";&&" delimiter
char** extract_commands(char *line)
{
	int bufsize =BUFSIZE2, position=0,i=0,delim_counter=0;
	char **tokens = malloc(bufsize*sizeof(char*));
	char *token;

	if (!tokens)
	{
	   fprintf(stderr, "myshell: allocation error\n");
	   exit(EXIT_FAILURE);
	}
	while(line[i]!='\0')
	{
		if((line[i])==';')
		{
			delims[delim_counter]=';';
			delim_counter++;
		}
		if(line[i]=='&' && line[i+1]=='&')
		{
			delims[delim_counter]='&';
			delim_counter++;
		}
		i++;
	}

	  token = strtok(line,DELIM);
	  while (token != NULL)
	  {
	    tokens[position] = token;
	    position++;
	    if (position >= bufsize)
	    {
	      bufsize += BUFSIZE2;
	      tokens = realloc(tokens, bufsize * sizeof(char*));
	      if (!tokens)
	      {
	        fprintf(stderr, "myshell: allocation error\n");
	        exit(EXIT_FAILURE);
	      }
	    }
	    token = strtok(NULL,DELIM);
	  }
	  tokens[position] = NULL;
	  return tokens;
}


//Tokenize command into arguments.
//Default arg buffer: 64. Reallocate for more.
// Args delimiter: " \t\r\n\a"
char **extract_args(char *commands)
{
  int bufsize = ARGS_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "myshell: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(commands, ARGS_DELIM);
  while (token != NULL)
  {
    tokens[position] = token;
    position++;

    if (position >= bufsize)
    {
      bufsize += ARGS_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens)
      {
        fprintf(stderr, "myshell: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, ARGS_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}


//Execute the command. Check if it is an empty command or a builtin.
//For the rest of commands create child process for the execution
//and have the parent wait. Status variable is returned
int execute_cmd(char **args)
{
  pid_t pid;
  int status;

  // An empty command was entered.
  if (args[0] == NULL)
  {
     return 0;
  }
  //Check for builtin
  for(int i=0; i<total_builtin_cmds(); i++)
  {
	  if (strcmp(args[0], builtin_cmds[i]) == 0)
	  {
	     return (*builtin_func[i])(args);
	  }
  }
  pid = fork();
  if (pid == 0)
  {
    // Child process
    if (execvp(args[0], args) == -1)
    {
      perror("myshell");

    }
    exit(EXIT_FAILURE);  //EXIT_FAILURE IS RETURNED TO PARENT PROCESS status value!
  }
  else if (pid < 0)
  {
    // Error forking
    perror("myshell");
  } else
  {
    // Parent process
	 do
	 {
		waitpid(pid, &status, WUNTRACED);
	 }while(!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return status;
}

//Calculate number of builtin commands
int total_builtin_cmds()
{
  return sizeof(builtin_cmds) / sizeof(char *);
}

//Change directory implementation
int builtin_cd(char **args)
{
 if (args[1] == NULL)
 {
   fprintf(stderr, "myshell: expected argument to \"cd\"\n");
 }
 else
 {
   if (chdir(args[1]) != 0)
   {
     perror("myshell");
   }
 }
 return 0;
}

//Quit Implementation
int builtin_exit(char **args)
{
  //Use 10 as status to symbolize that quit was entered
  return 10;
}


