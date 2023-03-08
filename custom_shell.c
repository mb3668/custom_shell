#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <grp.h>

void sig_handler(int signum) {
	FILE* log = NULL;
	char* fileName = "log/audit.log";

	log = fopen(fileName, "w");
	// FIXME. Write list of commands to file using fprintf(log, "%s", linked_list);
	fclose(log);
	kill(signum, SIGSTOP);
}

int errorCheck(int val, const char *str)
{
	if (val == -1)
	{
		perror(str);
		exit(EXIT_FAILURE);
	}
	else
	{
		return val;
	}
}

int main(int argc, char* argv[])
{
	//const char* command = argv[1]; // Pass in command
	char* mycmd[50] = {}; // Append command and arguments
	const char* command;
	char* inputFile;
	char* history[50] = {}; // Append history
	char user_input[200];

	// TEST
	char s[100];
	int checkAmp = 0; // Check for ampersand
	int checkLess = 0; // Check for <
	int checkGreater = 0; // Check for >
	int historyCounter = 0;

	// Use for tokenizing string
	int i = 0;

	// Register signal handler
	signal(SIGUSR1, sig_handler);

	printf("PID: < %d >\n", getpid());

	// Run while loop
	while(1) {
		printf("%s$", getcwd(s, 100)); // Print directory
		scanf(" %[^\n]s", user_input); // gets user input
		char* token = strtok(user_input, " "); // tokenize input into seperate strings
		size_t arrLength;

		// Place each string into mycmd
		while(token != NULL)
		{
			// Check to see if &, <, > is passed into commands
			// This will make sure that the token is not passed into arguments
			if (token == "&")
			{
				checkAmp = 1; // Check for &
			}
			else if (token == "<")
			{
				checkLess = 1; // Check for <
			}
			else if (token == ">")
			{
				checkGreater = 1; // Check for >
			}
			// Not finished
			else if (checkLess || checkGreater)
			{
				inputFile = token;
				checkLess = 0;
				checkGreater = 0;
			}
			else
			{
				mycmd[i] = token;
				i++;
			}

			token = strtok(NULL, " ");
		}

		// FIXME. Save history to list
		if (historyCounter < 10)
		{
			history[historyCounter] = mycmd[0];
			history[historyCounter+1] = '\0';
			//printf("%s\n", history[historyCounter]); // Check passed inputs
			historyCounter++; // Find out if more than 10 commands
		}
		else
		{
			// If more than 10 commands remove first
			for (int i = 0; i<10; i++)
			{
				history[i] = history[i+1];
			}
			history[9] = mycmd[0]; // Fill final index
		}

		mycmd[i+1] = '\0'; // Final command is NULL
		arrLength = i;
		i = 0; // Reset i each while iteration
		command = mycmd[0]; // Sets first input as the command to run

		// Check for internal commands
		if (strcmp(command, "cd") == 0)
		{
			// Change directory
			errorCheck(chdir(mycmd[1]), "chdir");
		}
		else if (strcmp(command, "history") == 0)
		{
			// History not finished
			for(int i = 0; i < historyCounter; i++)
			{
				printf("%s\n", history[i]);
			}
		}
		// Print environment variables. Not sure if this works on WSL just returns null
		else if (strcmp(command, "env") == 0)
		{
			printf("<%s>(<%d>)\n", getlogin(), getuid());
			printf("(<%d>)\n", getgid());
		}

		// If not an internal command run fork and execvp
		else
		{
			// Fork
			int pid = errorCheck(fork(), "fork");

			switch(pid)
			{
				case 0:
					// Implement exec functions
					errorCheck(execvp(command, mycmd), "execvp");

					exit(EXIT_SUCCESS);
				case -1:
					// Fork failure case
					exit(EXIT_FAILURE);
				default:
					// Parent function
					if (!checkAmp) wait(NULL); // If & found then do not wait
					checkAmp = 0;

					// Reset mycmd
					for(int i = 0; i<arrLength; i++)
					{
						memset(mycmd[i], '\0', strlen(mycmd[i]));
					}
					break;
			}
		}
	}

	exit(EXIT_SUCCESS);
}
