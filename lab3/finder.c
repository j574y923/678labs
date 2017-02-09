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
	
	/* ports for the pipes */
	int fd_a[2], fd_b[2], fd_c[2];
	/* pipe between first and second child with fd_a as the ports */
	pipe(fd_a);
	/* pipe between second and third child with fd_b as the ports */
	pipe(fd_b);
	/* pipe between third and fourth child with fd_c as the ports */
	pipe(fd_c);

	pid_1 = fork();
	if (pid_1 == 0) {
		/* First Child */
 		printf("FIRST CHILD\n");
		char cmdbuf[BSIZE];
 		bzero(cmdbuf, BSIZE);
 		sprintf(cmdbuf, "%s %s -name \'*\'.[ch]", FIND_EXEC, argv[1]);
		
		char *const arr[] = {BASH_EXEC, "-c", cmdbuf, (char *) 0};

		// for output
		dup2(fd_a[WRITE_END], STDOUT_FILENO);//stdout == 1
		// close unused unput half of pipe
		close(fd_a[READ_END]);

		/* close unused pipe between second and third child */
		close(fd_b[READ_END]);
		close(fd_b[WRITE_END]);
		/* close unused pipe between third and fourth child */
		// close(fd_c[READ_END]);
		// close(fd_c[WRITE_END]);

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
		
		// for input
		dup2(fd_a[READ_END], STDIN_FILENO);//stdin == 0
		// close unused half of pipe
		close(fd_a[WRITE_END]);

		// for output
		dup2(fd_b[WRITE_END], STDOUT_FILENO);
		// close unused half of pipe
		close(fd_b[READ_END]);

		/* close unused pipe between third and fourth child */
		// close(fd_c[READ_END]);
		// close(fd_c[WRITE_END]);

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
 		char cmdbuf[BSIZE];
 		bzero(cmdbuf, BSIZE);
 		sprintf(cmdbuf, "%s -t : +1.0 -2.0 --numeric --reverse ", SORT_EXEC);

 		char *const arr[] = {BASH_EXEC, "-c", cmdbuf, (char *) 0};

		// for input
		dup2(fd_b[READ_END], STDIN_FILENO);//stdin == 0
		// close unused half of pipe
		close(fd_b[WRITE_END]);

		// for output
		// dup2(fd_c[WRITE_END], STDOUT_FILENO);
		// // close unused half of pipe
		// close(fd_c[READ_END]);

		/* close unused pipe between first and second child */
		close(fd_a[READ_END]);
		close(fd_a[WRITE_END]);

 		if ( execv(BASH_EXEC, arr) < 0) {
 			fprintf(stderr, "\nError execing find. ERROR#%d\n", errno);
 			return EXIT_FAILURE;
		}

		exit(0);
	}

	pid_4 = fork();
	if (pid_4 == 0) {
		/* Fourth Child */
 		printf("FOURTH CHILD\n");
		exit(0);
	}

	//
	// IMPORTANT: these pipes must be closed off here, the place where the parent process occurs
	//
	close(fd_a[READ_END]);
	close(fd_a[WRITE_END]);
	close(fd_b[READ_END]);
	close(fd_b[WRITE_END]);
	// close(fd_c[READ_END]);
	// close(fd_c[WRITE_END]);

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
