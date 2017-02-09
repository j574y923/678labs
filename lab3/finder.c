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

/*
 * main
 *
 * Description	:	Function that will fork four child processes
 * @argc		:	Supplied by command line, should be 4
 * @argv 		:	Supplied by command line, should be formatted like DIR STR NUM_FILES
 *
 * $int 		:	Return integer specifying program exit status
 */
int main(int argc, char *argv[])
{
	/* Declare status of pid */
	int status;
	/* Declare pid's to track forked child processes */
	pid_t pid_1, pid_2, pid_3, pid_4;

	if (argc != 4) {
		printf("usage: finder DIR STR NUM_FILES\n");
		exit(0);
	}
	
	/* Declare ports for the pipes */
	int fd_a[2], fd_b[2], fd_c[2];
	/* Establish pipe between first and second child with fd_a as the ports */
	pipe(fd_a);
	/* Establish pipe between second and third child with fd_b as the ports */
	pipe(fd_b);
	/* Establish pipe between third and fourth child with fd_c as the ports */
	pipe(fd_c);

	/* Fork the first child process with pid == 0 which will execute a find command */
	pid_1 = fork();
	if (pid_1 == 0) {
		/* First Child */

		/* Open output end of the pipe between first and second child */
		dup2(fd_a[WRITE_END], STDOUT_FILENO);//stdout == 1
		/* Close unused half of pipe */
		close(fd_a[READ_END]);

		/* Close unused pipe between second and third child */
		close(fd_b[READ_END]);
		close(fd_b[WRITE_END]);
		/* Close unused pipe between third and fourth child */
		close(fd_c[READ_END]);
		close(fd_c[WRITE_END]);

		/* Declare a char array of size BSIZE that will store the command string */
		char cmdbuf[BSIZE];
		/* Set the first BSIZE bytes of the char array cmdbuf to 0 (i.e. alloc memory) */
 		bzero(cmdbuf, BSIZE);
 		/* Format the command string which finds in the directory argv[1] any .c or .h file */
 		sprintf(cmdbuf, "%s %s -name \'*\'.[ch]", FIND_EXEC, argv[1]);
		
		char *const arr[] = {BASH_EXEC, "-c", cmdbuf, (char *) 0};

		if ( execv(BASH_EXEC, arr) < 0) {
 			fprintf(stderr, "\nError execing find. ERROR#%d\n", errno);
 			return EXIT_FAILURE;
		}
		
		exit(0);
	}

	/* Fork the second child process with pid == 0 which will execute a grep command */
	pid_2 = fork();
	if (pid_2 == 0) {
		/* Second Child */		

		/* Open input end of the pipe between first and second child */
		dup2(fd_a[READ_END], STDIN_FILENO);//stdin == 0
		/* Close unused half of pipe */
		close(fd_a[WRITE_END]);

		/* Open output end of the pipe between second and third child */
		dup2(fd_b[WRITE_END], STDOUT_FILENO);
		/* Close unused half of pipe */
		close(fd_b[READ_END]);

		/* Close unused pipe between third and fourth child */
		close(fd_c[READ_END]);
		close(fd_c[WRITE_END]);

		char cmdbuf[BSIZE];
 		bzero(cmdbuf, BSIZE);
 		/* Format the command string which counts the number of occurrences of argv[2] in the files listed from the pipe */
 		sprintf(cmdbuf, "%s %s -c %s ", XARGS_EXEC, GREP_EXEC, argv[2]);

 		char *const arr[] = {BASH_EXEC, "-c", cmdbuf, (char *) 0};

		if ( execv(BASH_EXEC, arr) < 0) {
 			fprintf(stderr, "\nError execing grep. ERROR#%d\n", errno);
 			return EXIT_FAILURE;
		}

		exit(0);
	}

	/* Fork the third child process with pid == 0 which will execute a sort command */
	pid_3 = fork();
	if (pid_3 == 0) {
		/* Third Child */	

		/* Open input end of the pipe between second and third child */
		dup2(fd_b[READ_END], STDIN_FILENO);
		/* Close unused half of pipe */
		close(fd_b[WRITE_END]);

		/* Open output end of the pipe between third and fourth child */
		dup2(fd_c[WRITE_END], STDOUT_FILENO);
		/* Close unused half of pipe */
		close(fd_c[READ_END]);

		/* Close unused pipe between first and second child */
		close(fd_a[READ_END]);
		close(fd_a[WRITE_END]);

 		char cmdbuf[BSIZE];
 		bzero(cmdbuf, BSIZE);
 		/* Format the command string which sorts the occurrences numerically then reverses them from the pipe */
 		sprintf(cmdbuf, "%s -t : +1.0 -2.0 --numeric --reverse ", SORT_EXEC);

 		char *const arr[] = {BASH_EXEC, "-c", cmdbuf, (char *) 0};

 		if ( execv(BASH_EXEC, arr) < 0) {
 			fprintf(stderr, "\nError execing sort. ERROR#%d\n", errno);
 			return EXIT_FAILURE;
		}

		exit(0);
	}

	/* Fork the fourth child process with pid == 0 which will execute a head command */
	pid_4 = fork();
	if (pid_4 == 0) {
		/* Fourth Child */

		/* Open input end of the pipe between third and fourth child */
		dup2(fd_c[READ_END], STDIN_FILENO);
		/* Close unused half of pipe */
		close(fd_c[WRITE_END]);

		/* Close unused pipe between first and second child */
		close(fd_a[READ_END]);
		close(fd_a[WRITE_END]);
		/* Close unused pipe between second and third child */
		close(fd_b[READ_END]);
		close(fd_b[WRITE_END]);

 		char cmdbuf[BSIZE];
 		bzero(cmdbuf, BSIZE);
 		/* Format the command string which gets the first argv[3] lines of output from the pipe */
 		sprintf(cmdbuf, "%s --lines=%s ", HEAD_EXEC, argv[3]);

 		char *const arr[] = {BASH_EXEC, "-c", cmdbuf, (char *) 0};

		if ( execv(BASH_EXEC, arr) < 0) {
 			fprintf(stderr, "\nError execing head. ERROR#%d\n", errno);
 			return EXIT_FAILURE;
		}

		exit(0);
	}

	/* IMPORTANT: These pipes must be closed off here. This is the place where the parent process occurs */
	close(fd_a[READ_END]);
	close(fd_a[WRITE_END]);
	close(fd_b[READ_END]);
	close(fd_b[WRITE_END]);
	close(fd_c[READ_END]);
	close(fd_c[WRITE_END]);

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
