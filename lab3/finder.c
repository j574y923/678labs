#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <sys/wait.h>

#define BSIZE 256

#define BASH_EXEC  "/bin/bash"
#define FIND_EXEC  "/bin/find"
#define XARGS_EXEC "/usr/bin/xargs"
#define GREP_EXEC  "/bin/grep"
#define SORT_EXEC  "/bin/sort"
#define HEAD_EXEC  "/usr/bin/head"

#define READ_END 0
#define WRITE_END 1

int main(int argc, char *argv[])
{
	int status;
	pid_t pid_1, pid_2, pid_3, pid_4;

	if (argc != 4) {
		printf("usage: finder DIR STR NUM_FILES\n");
		exit(0);
	}
	
	int fd[2];
	pipe(fd);

	pid_1 = fork();
	if (pid_1 == 0) {
		/* First Child */
 		printf("FIRST CHILD\n");
		char cmdbuf[BSIZE];
 		bzero(cmdbuf, BSIZE);
 		sprintf(cmdbuf, "%s %s -name \'*\'.[ch]", FIND_EXEC, argv[1]);
		
		char *const arr[] = {BASH_EXEC, "-c", cmdbuf, (char *) 0};

		dup2(fd[1], STDOUT_FILENO);//stdout == 1
		// close unused unput half of pipe
		close(fd[0]);

		if ( execv(BASH_EXEC, arr) < 0) {
 			fprintf(stderr, "\nError execing find. ERROR#%d\n", errno);
 			return EXIT_FAILURE;
		}
		
		exit(0);
	}

	pid_2 = fork();
	if (pid_2 == 0) {
		/* Second Child */
 		printf("SECOND CHILD\n");
		char cmdbuf[BSIZE];
 		bzero(cmdbuf, BSIZE);
 		sprintf(cmdbuf, "%s %s -c %s ", XARGS_EXEC, GREP_EXEC, argv[2]);

 		char *const arr[] = {BASH_EXEC, "-c", cmdbuf, (char *) 0};
		
		dup2(fd[0], STDIN_FILENO);//stdin == 0
		// close unused half of pipe
		close(fd[1]);

		if ( execv(BASH_EXEC, arr) < 0) {
 			fprintf(stderr, "\nError execing find. ERROR#%d\n", errno);
 			return EXIT_FAILURE;
		}

		exit(0);
	}

	pid_3 = fork();
	if (pid_3 == 0) {
		/* Third Child */
 		printf("THIRD CHILD\n");
		exit(0);
	}

	pid_4 = fork();
	if (pid_4 == 0) {
		/* Fourth Child */
 		printf("FOURTH CHILD\n");
		exit(0);
	}


	close(fd[0]);
	close(fd[1]);

	if ((waitpid(pid_1, &status, 0)) == -1) {
		fprintf(stderr, "Process 1 encountered an error. ERROR%d", errno);
		return EXIT_FAILURE;
	}
	if ((waitpid(pid_2, &status, 0)) == -1) {
		fprintf(stderr, "Process 2 encountered an error. ERROR%d", errno);
		return EXIT_FAILURE;
	}
	if ((waitpid(pid_3, &status, 0)) == -1) {
		fprintf(stderr, "Process 3 encountered an error. ERROR%d", errno);
		return EXIT_FAILURE;
	}
	if ((waitpid(pid_4, &status, 0)) == -1) {
		fprintf(stderr, "Process 4 encountered an error. ERROR%d", errno);
		return EXIT_FAILURE;
	}

	return 0;
}
