//Shell

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>      //for file i/o
#include <fcntl.h>       //for file i/o

#define CMD_LENGTH 512



int getNumArgs(char* line);

void getArgs(char* line, char* args[], int argNum);

int executeCD(char* args[], int numArgs);

int executePWD(char* args[]);

void compileCProgram(char* args[], pid_t pid, char* progName);

void runCProg(char* name);




int main(int argc, char **argv) {


	//buffer is to hold the commands that the user will type in
	char error_message[30] = "An error has occurred\n";
	char buffer[CMD_LENGTH];
	char bufferCpy[CMD_LENGTH];
	char bufferCpy2[CMD_LENGTH];

	// /bin/program_name is the arguments to pass to execv
	//if we want to run ls, "/bin/ls" is required to be passed to execv()
	char* prompt = "mysh>";
	char* path = "/bin/";
	char* outfile = NULL;
	char* nullString = NULL;

	//if incorrect number of args
	if (argc != 1) {
		if (argc != 2) {
			//bad cmd line
			write(STDERR_FILENO, error_message, strlen(error_message));
			exit(1);
		}
	}

	if (argv[1] != NULL) {
		FILE* infile = fopen(argv[1], "r");
		if (infile == NULL) {
			write(STDERR_FILENO, error_message, strlen(error_message));
			exit(1);
		}
		else {
			int i;
			for (i = 0; i < CMD_LENGTH; i++) {
				buffer[i] = ' ';
			}
			while ((fgets(buffer, strlen(buffer), infile)) != NULL) {
				if (ferror(infile)) {
					write(STDERR_FILENO, error_message, strlen(error_message));
					exit(1);
				}

				if (buffer[0] == '\0') {
					exit(0);
				}


				int i;

				/*
				char ch;
				while ((ch = getchar()) == '\n') {
					write(STDERR_FILENO, error_message, strlen(error_message));
					exit(0);
				}
				*/

				//get input

				//if (buffer[CMD_LENGTH -1] == '\n') {
				//	buffer[CMD_LENGTH -1] = '\0';
				//}

				write(STDOUT_FILENO, buffer, strlen(buffer));

				/*
				if (strcmp(nullString, lineTest) == 0) {
					write(STDERR_FILENO, error_message, strlen(error_message));
				}
				*/


				//to determine if command is a compile-and-run
				int isCompileCommand = 1;

				//for loop iterator

				//number of redirect symbols; 1 is proper
				int redCount = 0;

				//determine number of redirect symbols
				for (i = 0; i < CMD_LENGTH; i++) {
					bufferCpy[i] = buffer[i];
					if (buffer[i] == '>') {
						redCount++;
					}
				}

				//if more than one redirect symbol



				//count number of arguments
				int numOfArgs = getNumArgs(buffer);

				//proper length of argument array (for NULL terminator)
				int argLen = numOfArgs + 1;

				// if redirection, discount output
				if (redCount == 1) {
					argLen = numOfArgs;
				}

				//arguments for programs
				char* arguments[argLen];

				//add arguments to array
				getArgs(bufferCpy, arguments, numOfArgs + 1);


				//if redirection, set outfile aside
				if (redCount == 1) {
					outfile = arguments[argLen - 1];
				}

				//add null terminator
				arguments[argLen - 1] = NULL;


				/*list args - for testing
				for (i = 0; i < argLen - 1; i++) {
					printf("args: %s\n", arguments[i]);
				}
				*/


				//program name (unless compile-and-run command)
				char* progName = arguments[0];

				//determines if command is compile-and-run, sets progName
				for (i = 0; i < argLen - 1; i++) {
					if (strcmp(arguments[i] + (strlen(arguments[i]) - 2), ".c") == 0) {
						isCompileCommand = 0;
						progName = arguments[i];
					}
				}


				//if cmnd is built in, execute
				if (strcmp(progName, "cd") == 0) {
					int retVal = 0;
					retVal = executeCD(arguments, numOfArgs);
					if (retVal == 1) {
						write(STDERR_FILENO, error_message, strlen(error_message));
					}
				}
				else if (strcmp(progName, "pwd") == 0) {
					if (arguments[1] != NULL) {
						write(STDERR_FILENO, error_message, strlen(error_message));
					}
					else {
						int retVal;
						retVal = executePWD(arguments);
						if (retVal == 1) {
							write(STDERR_FILENO, error_message, strlen(error_message));
						}
					}
				}
				else if (strcmp(progName, "exit") == 0) {
					if (arguments[1] != NULL) {
						write(STDERR_FILENO, error_message, strlen(error_message));
					}
					else {
						exit(0);
					}
				}

				//not built in command
				else {

					//fork new process
					pid_t cPid = fork();

					//make sure it's not parent
					if (cPid != 0) {
						wait(NULL);
					}

					//we're in the child
					else {

						if (redCount > 1) {
							write(STDERR_FILENO, error_message, strlen(error_message));
						}

						//if compile-and-run
						if (isCompileCommand == 0) {

							//structure args

							/*
							printf("???\n");
							char* nameCpy = NULL;
							printf("why!@");
							*nameCpy = *progName;
							printf("here: %s\n", nameCpy);
							char* tmp = strtok(nameCpy, ".c");
							printf("!!!\n");

							char* cmpArgs[5];
							cmpArgs[0] = "gcc";
							cmpArgs[1] = progName;
							cmpArgs[2] = "o";
							cmpArgs[3] = tmp;
							cmpArgs[4] = NULL;
							*/

							char* cmpArgs[3];
							cmpArgs[0] = "gcc";
							cmpArgs[1] = progName;
							cmpArgs[2] = NULL;

							//print args, for testing
							//for (i = 0; i < 3; i++) {
							//	printf("args: %s\n", cmpArgs[i]);
							//}

							//compile program
							compileCProgram(cmpArgs, cPid, "a.out");
						}

						//holds prog location and name for execvp
						char progInfo[CMD_LENGTH];

						
						strcpy(progInfo, progName);
						
		
						//if redirection, switch output to outfile
						if (redCount == 1) {
							int fd = open(outfile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
							if (fd < 0) {
								write(STDERR_FILENO, error_message, strlen(error_message));
								exit(0);
							}
							else {
								dup2(fd, 1);
							}
						}

						int retVal = execvp(progInfo, arguments);

						write(STDERR_FILENO, error_message, strlen(error_message));

					}
				}
			}
			return 0;
		}	
	}
			



	while(1) {



		//print the prompt
		write(STDOUT_FILENO, prompt, strlen(prompt));

		//INSERT BATCH MODE HERE!!!
		/*
		char ch;
		while ((ch = getchar()) == '\n') {
			write(STDERR_FILENO, error_message, strlen(error_message));
			exit(0);
		}
		*/

		//get input


		if (fgets(buffer, 512, stdin) == NULL) {
			write(STDERR_FILENO, error_message, strlen(error_message));
			exit(1);
		}


		/*
		if (strcmp(nullString, lineTest) == 0) {
			write(STDERR_FILENO, error_message, strlen(error_message));
		}
		*/


		//to determine if command is a compile-and-run
		int isCompileCommand = 1;

		//for loop iterator
		int i;

		//number of redirect symbols; 1 is proper
		int redCount = 0;

		//determine number of redirect symbols
		for (i = 0; i < CMD_LENGTH; i++) {
			bufferCpy[i] = buffer[i];
			if (buffer[i] == '>') {
				redCount++;
			}
		}

		//if more than one redirect symbol
		if (redCount > 1) {
			write(STDERR_FILENO, error_message, strlen(error_message));
		}



		//count number of arguments
		int numOfArgs = getNumArgs(buffer);

		//proper length of argument array (for NULL terminator)
		int argLen = numOfArgs + 1;

		// if redirection, discount output
		if (redCount == 1) {
			argLen = numOfArgs;
		}

		//arguments for programs
		char* arguments[argLen];

		//add arguments to array
		getArgs(bufferCpy, arguments, numOfArgs + 1);


		//if redirection, set outfile aside
		if (redCount == 1) {
			outfile = arguments[argLen - 1];
		}

		//add null terminator
		arguments[argLen - 1] = NULL;


		/*list args - for testing
		for (i = 0; i < argLen - 1; i++) {
			printf("args: %s\n", arguments[i]);
		}
		*/


		//program name (unless compile-and-run command)
		char* progName = arguments[0];

		//determines if command is compile-and-run, sets progName
		for (i = 0; i < argLen - 1; i++) {
			if (strcmp(arguments[i] + (strlen(arguments[i]) - 2), ".c") == 0) {
				isCompileCommand = 0;
				progName = arguments[i];
			}
		}


		//if cmnd is built in, execute
		if (strcmp(progName, "cd") == 0) {
			int retVal = 0;
			retVal = executeCD(arguments, numOfArgs);
			if (retVal == 1) {
				write(STDERR_FILENO, error_message, strlen(error_message));
			}
		}
		else if (strcmp(progName, "pwd") == 0) {
			if (arguments[1] != NULL) {
				write(STDERR_FILENO, error_message, strlen(error_message));
			}
			else {
				int retVal;
				retVal = executePWD(arguments);
				if (retVal == 1) {
					write(STDERR_FILENO, error_message, strlen(error_message));
				}
			}
		}
		else if (strcmp(progName, "exit") == 0) {
			if (arguments[1] != NULL) {
				write(STDERR_FILENO, error_message, strlen(error_message));
			}
			else {
				exit(0);
			}
		}


		//not built in command
		else {

			//fork new process
			pid_t cPid = fork();

			//make sure it's not parent
			if (cPid != 0) {
				wait(NULL);
			}

			//we're in the child
			else {

				//if compile-and-run
				if (isCompileCommand == 0) {

					//structure args

					/*
					printf("???\n");
					char* nameCpy = NULL;
					printf("why!@");
					*nameCpy = *progName;
					printf("here: %s\n", nameCpy);
					char* tmp = strtok(nameCpy, ".c");
					printf("!!!\n");

					char* cmpArgs[5];
					cmpArgs[0] = "gcc";
					cmpArgs[1] = progName;
					cmpArgs[2] = "o";
					cmpArgs[3] = tmp;
					cmpArgs[4] = NULL;
					*/

					char* cmpArgs[3];
					cmpArgs[0] = "gcc";
					cmpArgs[1] = progName;
					cmpArgs[2] = NULL;

					//print args, for testing
					for (i = 0; i < 3; i++) {
						printf("args: %s\n", cmpArgs[i]);
					}

					//compile program
					compileCProgram(cmpArgs, cPid, "a.out");
				}

				//holds prog location and name for execvp
				char progInfo[CMD_LENGTH];


				strcpy(progInfo, progName);
				

				//adds program path and name
				//strcpy(progInfo, path);

				//if redirection, switch output to outfile
				if (redCount == 1) {
					int fd = open(outfile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
					if (fd < 0) {
						write(STDERR_FILENO, error_message, strlen(error_message));
						exit(0);
					}
					else {
						dup2(fd, 1);
					}
				}

				int retVal = execvp(progInfo, arguments);

				write(STDERR_FILENO, error_message, strlen(error_message));

			}
		}
	}
	return 0;
}



int getNumArgs(char* line) {
	int count = 0;
	char* ptr = strtok(line, " \t>");

	while (ptr != NULL) {
		count++;
		ptr = strtok(NULL, " \t>");
	}
	return count;
}


void getArgs(char* line, char* args[], int argNum) {
	char* brk;
	int i;

	for (i = 0; i < (argNum - 1); i++) {
		if (i == 0) {
			brk = strtok(line, " \n\t>");
		}
		else {
			brk = strtok(NULL, " \n\t>");
		}
		args[i] = brk;
	}
}



int executeCD(char* args[], int numArgs){

	if(numArgs > 2){
		return 1;
	}

	else if (numArgs == 1) {
		int retVal = chdir(getenv("HOME"));
		if (retVal != 0) {
			return 1;
		}
	}

	else {
		int retVal = chdir(args[1]);
		if (retVal != 0) {
			return 1;
		}
	}
	return 0;
}

int executePWD(char* args[]) {
	char buffer[CMD_LENGTH];
	char* retVal = getcwd(buffer, CMD_LENGTH);
	if (retVal == NULL) {
		return 1;
	}
	write(STDOUT_FILENO, buffer, strlen(buffer));
	write(STDOUT_FILENO, "\n", strlen("\n"));
	return 0;
}



void compileCProgram(char* args[], pid_t pid, char* progName) {

	pid_t pid3;
	int status;

	//create fork here for compilation
	//int pid2 = fork();

	//if (pid2 == 0) {
		char progInfo[CMD_LENGTH];
		strcpy(progInfo, "/usr/bin/");
		strcat(progInfo, "gcc");
		int retVal = execvp(progInfo, args);
	//}

		/*
	while ((pid3 = waitpid(-1, &status, 0)) > 0) {
		char runInfo[CMD_LENGTH];
		char* runArgs[2];
		runArgs[0] = progName;
		runArgs[1] = NULL;
		strcpy(runInfo, "/bin/");
		strcat(runInfo, "./");

		int retVal = execvp(runInfo, runArgs);
	}
	*/



	/*

	int pid2 = fork();

	if (pid2 != 0) {
		waitpid(-1, &pid, WNOHANG);
	}
	else {
		char runInfo[CMD_LENGTH];
		char* runArgs[2];
		runArgs[0] = progName;
		runArgs[1] = NULL;
		strcpy(progInfo, "/bin/");
		strcat(progInfo, "./");

		int retVal = execvp(runInfo, runArgs);
	}
	*/

}



void runCProg(char* name) {

	int pid = fork();

	if (pid != 0) {
		wait(NULL);
	}

	else {

		char runInfo[CMD_LENGTH];
		char* runArgs[2];
		runArgs[0] = name;
		runArgs[1] = NULL;
		strcpy(runInfo, "/bin/");
		strcat(runInfo, "./");

		int retVal = execvp(runInfo, runArgs);
	}
}




















