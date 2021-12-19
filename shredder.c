#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>

#define ERR1LEN 39
#define ERR2LEN 66
#define SHREDDERLEN 16
#define BUFFERLEN 1024
#define TRUE 1
#define FALSE 0
#define BWAHALEN 42

int pid;
int sigint_sent = FALSE;

void handle_sig(int sig) {
	//handles signal
	if (sig == SIGINT) {
		write(STDOUT_FILENO, "\npenn-shredder# ", SHREDDERLEN);
		sigint_sent = TRUE;
	} else if (sig == SIGALRM) {
		write(STDOUT_FILENO, "Bwahaha ... tonight I dine on turtle soup\n", BWAHALEN);
		kill(pid, SIGALRM);
	}
}

int main(int argc, const char* argv[]) {
	int timer = 0;
	int valid0 = FALSE;

	if (argc == 2) { // set timer
		if (strcmp(argv[1],"0") == 0) {
			valid0 = TRUE;
		}
		timer = atoi(argv[1]);
	}

	if (timer <= 0 && !valid0) { // makes sure timer exists/is valid
		write(STDOUT_FILENO, "Invalid: Missing or invalid timer argument! Running with no timer\n", ERR2LEN);
	}

	signal(SIGALRM, handle_sig); // for sigalrm
	signal(SIGINT, handle_sig); // for sigint

	write(STDOUT_FILENO, "penn-shredder# ", SHREDDERLEN);

	while (TRUE) {
		char* buf = malloc(BUFFERLEN);
		int reading = read(STDIN_FILENO, buf, BUFFERLEN);

		if (reading == 0) { // handles EOF
			write(STDOUT_FILENO, "\n", 1);
			free(buf);
			exit(0);
		}

		if (buf[0] == '\0') { // continue if nothing read
			free(buf);
			continue;
		} else if (buf[0] == '\n') { // reprompt if user just presses enter
			write(STDOUT_FILENO, "penn-shredder# ", SHREDDERLEN);
			free(buf);
			continue;
		}

		// truncate and flush inputs longer than 1024 characters
		if (reading >= 1024 && buf[1023] != '\n') {
			buf[1023] = '\0';

			char* clear_buf = malloc(BUFFERLEN);
			int read_clear = read(STDIN_FILENO, clear_buf, BUFFERLEN);
			while (read_clear > 1024) {
				read_clear = read(STDIN_FILENO, clear_buf, BUFFERLEN);
			}

			int length = strlen(buf);

			for (int i = 1024; i < length; i++) {
				buf[i] = '\0';
			}
		}

		// ensures child does not inherit the timer
		alarm(timer);

		pid = fork();

		if (pid == 0) { // if current process is the child process
			char** argv = malloc(2*sizeof(char*));

			argv[0] = strtok(buf, " \n");
			argv[1] = NULL;

			char** envp = {NULL};

			int execval = execve(argv[0], argv, envp); // execute command

			free(buf);
			free(argv);

			alarm(0); // cancel timer if an error occurred

			if (execval == -1) {
				if (errno == 2) {
					perror("");
				} else {
					perror("Invalid");
				}
			}

			exit(1);
		} else { // if current process is the parent process
			wait(&pid); // wait until child executes

			free(buf);

			alarm(0); // cancel timer

			if (!sigint_sent) {
				write(STDOUT_FILENO, "penn-shredder# ", SHREDDERLEN);
			}
			sigint_sent = FALSE;
		}
	}

	return 0;
}