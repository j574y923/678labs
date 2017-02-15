#include <stdio.h>     /* standard I/O functions                         */
#include <stdlib.h>    /* exit                                           */
#include <unistd.h>    /* standard unix functions, like getpid()         */
#include <signal.h>    /* signal name macros, and the signal() prototype */

#include <string.h>

/* first, define the Ctrl-C counter, initialize it with zero. */
int ctrl_c_count = 0;
int got_response = 0;
#define CTRL_C_THRESHOLD  5 

/* the Ctrl-C signal handler */
void catch_int(int sig_num)
{
  /* increase count, and check if threshold was reached */
  ctrl_c_count++;
  if (ctrl_c_count >= CTRL_C_THRESHOLD) {
    char answer[30];

    /* prompt the user to tell us if to really
     * exit or not */
    printf("\nReally exit? [Y/n]: ");
    fflush(stdout);

    got_response = 0;
    alarm(5);

    fgets(answer, sizeof(answer), stdin);
    if (answer[0] == 'n' || answer[0] == 'N') {

      got_response = 1;

      printf("\nContinuing\n");
      fflush(stdout);
      /* 
       * Reset Ctrl-C counter
       */
      ctrl_c_count = 0;
    }
    else {
      printf("\nExiting...\n");
      fflush(stdout);
      exit(0);
    }
  }
}

/* the Ctrl-Z signal handler */
void catch_tstp(int sig_num)
{
  /* print the current Ctrl-C counter */
  printf("\n\nSo far, '%d' Ctrl-C presses were counted\n\n", ctrl_c_count);
  fflush(stdout);
}

void catch_alrm()
{
  if(got_response == 0){
    printf("\nUser taking too long to respond. Exiting  . . .\n");
    fflush(stdout);
    exit(0);
  }
}

int main(int argc, char* argv[])
{
  struct sigaction sa;
  sigset_t mask_set;  /* used to set a signal masking set. */

  /* setup mask_set */
  memset (&sa, '\0', sizeof(sa));
  sigfillset(&mask_set);
  sigdelset(&mask_set, SIGALRM);
  sa.sa_mask = mask_set;

  /* set signal handlers */
  sa.sa_handler = catch_int;
  if (sigaction(SIGINT, &sa, NULL) < 0) {
    perror ("SIGINT");
    return 1;
  }

  sa.sa_handler = catch_tstp;
  if (sigaction(SIGTSTP, &sa, NULL) < 0) {
    perror ("SIGTSTP");
    return 1;
  }

  sa.sa_handler = catch_alrm;
  if (sigaction(SIGALRM, &sa, NULL) < 0) {
    perror ("SIGALRM");
    return 1;
  }

  while(1){
    pause();
  }

  return 0;
}

