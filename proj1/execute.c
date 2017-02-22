/**
 * @file execute.c
 *
 * @brief Implements interface functions between Quash and the environment and
 * functions that interpret an execute commands.
 *
 * @note As you add things to this file you may want to change the method signature
 */

#include "execute.h"

#include <stdio.h>

#include "quash.h"

#include <sys/wait.h>

#include <sys/stat.h>

#include <fcntl.h>

#include <unistd.h>

#include "pid_queue.h"

int idx;
int idx_num;
// Remove this and all expansion calls to it
/**
 * @brief Note calls to any function that requires implementation
 */
#define IMPLEMENT_ME()                                                  \
  fprintf(stderr, "IMPLEMENT ME: %s(line %d): %s()\n", __FILE__, __LINE__, __FUNCTION__)

/***************************************************************************
 * Interface Functions
 ***************************************************************************/

// Return a string containing the current working directory.
char* get_current_directory(bool* should_free) {
  // TODO: Get the current working directory. This will fix the prompt path.
  // HINT: This should be pretty simple
  //IMPLEMENT_ME();
  char *buf;

  buf=(char *)malloc(100*sizeof(char));
  getcwd(buf,1024);

  //printf("current_directory:%s\n",buf);
  //free(buf);
  // Change this to true if necessary
  *should_free = true;

  return buf;
}

// Returns the value of an environment variable env_var
const char* lookup_env(const char* env_var) {
  // TODO: Lookup environment variables. This is required for parser to be able
  // to interpret variables from the command line and display the prompt
  // correctly
  // HINT: This should be pretty simple
  //IMPLEMENT_ME();
  
  // TODO: Remove warning silencers
  (void) env_var; // Silence unused variable warning

  return getenv(env_var);
}

// Check the status of background jobs
void check_jobs_bg_status() {
  // TODO: Check on the statuses of all processes belonging to all background
  // jobs. This function should remove jobs from the jobs queue once all
  // processes belonging to a job have completed.
  //IMPLEMENT_ME();

  // TODO: Once jobs are implemented, uncomment and fill the following line
  // print_job_bg_complete(job_id, pid, cmd);
}

// Prints the job id number, the process id of the first process belonging to
// the Job, and the command string associated with this job
void print_job(int job_id, pid_t pid, const char* cmd) {
  printf("[%d]\t%8d\t%s\n", job_id, pid, cmd);
  fflush(stdout);
}

// Prints a start up message for background processes
void print_job_bg_start(int job_id, pid_t pid, const char* cmd) {
  printf("Background job started: ");
  print_job(job_id, pid, cmd);
}

// Prints a completion message followed by the print job
void print_job_bg_complete(int job_id, pid_t pid, const char* cmd) {
  printf("Completed: \t");
  print_job(job_id, pid, cmd);
}

/***************************************************************************
 * Functions to process commands
 ***************************************************************************/
// Run a program reachable by the path environment variable, relative path, or
// absolute path
void run_generic(GenericCommand cmd) {
  // Execute a program with a list of arguments. The `args` array is a NULL
  // terminated (last string is always NULL) list of strings. The first element
  // in the array is the executable
  char* exec = cmd.args[0];
  char** args = cmd.args;

  // TODO: Remove warning silencers
  //(void) exec; // Silence unused variable warning
  //(void) args; // Silence unused variable warning

  // TODO: Implement run generic
  //IMPLEMENT_ME();
  execvp(exec, args);
  //perror("ERROR: Failed to execute program");
}

// Print strings
void run_echo(EchoCommand cmd) {
  // Print an array of strings. The args array is a NULL terminated (last
  // string is always NULL) list of strings.
  char** str = cmd.args;

  // TODO: Remove warning silencers
  //(void) str; // Silence unused variable warning

  // TODO: Implement echo
  //IMPLEMENT_ME();
  while(*str != NULL){
    printf("%s ",*str);
    str++;
  }  
  printf("\n");
  // Flush the buffer before returning
  fflush(stdout);
}

// Sets an environment variable
void run_export(ExportCommand cmd) {
  // Write an environment variable
  const char* env_var = cmd.env_var;
  const char* val = cmd.val;

  // TODO: Remove warning silencers
  (void) env_var; // Silence unused variable warning
  (void) val;     // Silence unused variable warning

  // TODO: Implement export.
  // HINT: This should be quite simple.
  //IMPLEMENT_ME();
  if(setenv(env_var, val, 1) != 0)
	  perror("ERROR: Failed to export environment variable\n");
}

// Changes the current working directory
void run_cd(CDCommand cmd) {
  // Get the directory name
  const char* dir = cmd.dir;
  
  // Check if the directory is valid
  if (dir == NULL) {
    perror("ERROR: Failed to resolve path");
    return;
  }

  // TODO: Change directory
  chdir(cmd.dir);
  // TODO: Update the PWD environment variable to be the new current working
  // directory and optionally update OLD_PWD environment variable to be the old
  // working directory.
  const char* old_dir = getenv("PWD");
  if(setenv("PWD", dir, 1) != 0)				//setenv returns -1 if unsuccessfull, 0 for success
	  perror("ERROR: Failed to update PWD");
  if(setenv("OLD_PWD", old_dir, 1) != 0)
	  perror("ERROR: Failed to update OLD_PWD"); 
  //IMPLEMENT_ME();
}

// Sends a signal to all processes contained in a job
void run_kill(KillCommand cmd) {
  int signal = cmd.sig;
  int job_id = cmd.job;

  // TODO: Remove warning silencers
  (void) signal; // Silence unused variable warning
  (void) job_id; // Silence unused variable warning

  // TODO: Kill all processes associated with a background job
  IMPLEMENT_ME();
}


// Prints the current working directory to stdout
void run_pwd() {
  // TODO: Print the current working directory
  //IMPLEMENT_ME();
  char *buf;
  //buf=(char *)malloc(100*sizeof(char));
  buf = (char*)get_current_dir_name();
  //getcwu(buf,1024);

  printf("%s\n",buf);
  // Flush the buffer before returning
  free(buf);
  fflush(stdout);
}

// Prints all background jobs currently in the job list to stdout
void run_jobs() {
  // TODO: Print background jobs
  IMPLEMENT_ME();

  // Flush the buffer before returning
  fflush(stdout);
}

/***************************************************************************
 * Functions for command resolution and process setup
 ***************************************************************************/

/**
 * @brief A dispatch function to resolve the correct @a Command variant
 * function for child processes.
 *
 * This version of the function is tailored to commands that should be run in
 * the child process of a fork.
 *
 * @param cmd The Command to try to run
 *
 * @sa Command
 */
void child_run_command(Command cmd) {
  CommandType type = get_command_type(cmd);

  switch (type) {
  case GENERIC:
    run_generic(cmd.generic);
    break;

  case ECHO:
    run_echo(cmd.echo);
    break;

  case PWD:
    run_pwd();
    break;

  case JOBS:
    run_jobs();
    break;

  case EXPORT:
  case CD:
  case KILL:
  case EXIT:
  case EOC:
    break;

  default:
    fprintf(stderr, "Unknown command type: %d\n", type);
  }
}

/**
 * @brief A dispatch function to resolve the correct @a Command variant
 * function for the quash process.
 *
 * This version of the function is tailored to commands that should be run in
 * the parent process (quash).
 *
 * @param cmd The Command to try to run
 *
 * @sa Command
 */
void parent_run_command(Command cmd) {
  CommandType type = get_command_type(cmd);

  switch (type) {
  case EXPORT:
    run_export(cmd.export);
    break;

  case CD:
    run_cd(cmd.cd);
    break;

  case KILL:
    run_kill(cmd.kill);
    break;

  case GENERIC:
  case ECHO:
  case PWD:
  case JOBS:
  case EXIT:
  case EOC:
    break;

  default:
    fprintf(stderr, "Unknown command type: %d\n", type);
  }
}

/**
 * @brief Creates one new process centered around the @a Command in the @a
 * CommandHolder setting up redirects and pipes where needed
 *
 * @note Processes are not the same as jobs. A single job can have multiple
 * processes running under it. This function creates a process that is part of a
 * larger job.
 *
 * @note Not all commands should be run in the child process. A few need to
 * change the quash process in some way
 *
 * @param holder The CommandHolder to try to run
 *
 * @sa Command CommandHolder
 */
void create_process(CommandHolder holder) {
  // Read the flags field from the parser
  bool p_in  = holder.flags & PIPE_IN;
  bool p_out = holder.flags & PIPE_OUT;
  bool r_in  = holder.flags & REDIRECT_IN;
  bool r_out = holder.flags & REDIRECT_OUT;
  bool r_app = holder.flags & REDIRECT_APPEND; // This can only be true if r_out
                                               // is true

  // TODO: Remove warning silencers
  (void) p_in;  // Silence unused variable warning
  (void) p_out; // Silence unused variable warning
  (void) r_in;  // Silence unused variable warning
  (void) r_out; // Silence unused variable warning
  (void) r_app; // Silence unused variable warning

  // TODO: Setup pipes, redirects, and new process
  //IMPLEMENT_ME();

  pid_t child_pid;
  int status; 				// Declare status of child pid

  //int fd_a[2];				// File descriptor for pipe handle
  //pipe(fd_a);

  int pipe_end = idx_num * 2;

  child_pid = fork();
  
  if(child_pid == 0){
 
    if(p_in && !p_out){
      printf("CHILD: P_IN: STDIN\n");	
      dup2(fd_a[idx * 2 - 2], STDIN_FILENO);	// Replace the standard input with pipe read end
      close(fd_a[idx * 2 - 1]);

      //close unused pipes
      for(int i = 0; i < idx * 2 - 2; ++i)
        close(fd_a[i]);
    } else if(p_out && !p_in){
      printf("CHILD: P_OUT: STDOUT\n");
      dup2(fd_a[1], STDOUT_FILENO);	// Replace the standard output with pipe write end
      close(fd_a[0]);

      //close unused pipes
      for(int i = 2; i < idx * 2; ++i)
        close(fd_a[i]);
    } else if (p_out && p_in){
      printf("CHILD: P_OUT && P_IN\n");
      dup2(fd_a[pipe_end-2], STDIN_FILENO);
      close(fd_a[pipe_end-1]);

      dup2(fd_a[pipe_end+1], STDOUT_FILENO);
      close(fd_a[pipe_end]);

      //close unused pipes
      for(int i = 0; i < pipe_end-2; ++i)
        close(fd_a[i]);
      for(int i = pipe_end+2; i < idx * 2; ++i)
        close(fd_a[i]);
    }
    
    if(r_in){
      int fp = open(holder.redirect_in, O_RDONLY);
      dup2(fp, 0);
      close(fp);
    }

    if(r_out){
      if(r_app){
        int fp = open(holder.redirect_out, O_RDWR | O_CREAT | O_APPEND, 0777);
        dup2(fp, 1);
        close(fp);
      }
      else{
        int fp = open(holder.redirect_out, O_RDWR | O_CREAT, 0777);
        dup2(fp, 1);
        close(fp);
      }
    }
    child_run_command(holder.cmd); // This should be done in the child branch 
    exit(0);
  }
  else{
    push_back_pid_queue (&process_queue, child_pid);	
    parent_run_command(holder.cmd); // This should be done in the parent branch of a fork
  }

  //close(fd_a[0]);
  //close(fd_a[1]);

  /*if ((waitpid(child_pid, &status, 0)) == -1) {
  	perror("Process 1 encountered an error");
  	return;
  }*/
}

// Run a list of commands
void run_script(CommandHolder* holders) {
  int status;

  if (holders == NULL)
    return;

  check_jobs_bg_status();

  if (get_command_holder_type(holders[0]) == EXIT &&
      get_command_holder_type(holders[1]) == EOC) {
    end_main_loop();
    return;
  }

  CommandType type;

  idx = 0;
  for (idx; (type = get_command_holder_type(holders[idx])) != EOC; ++idx){}
  printf("IDX???%d\n",idx);

  --idx;//represents the number of pipes
  if(idx < 1)
    idx = 1;

  fd_a = (int *)malloc(idx * 2 * sizeof(int));//need 2 ends for each pipe (denoted by idx)

  idx_num = 0;//keep track of which pipe we are at

  //make pipes
  for (int i = 0; i < idx; ++i) 
    pipe(&fd_a[i*2]);

  // Run all commands in the `holder` array
  for (int i = 0; (type = get_command_holder_type(holders[i])) != EOC; ++i){
    create_process(holders[i]);
    printf("IDX_NUM???%d\n",idx_num);
    ++idx_num;
  }

  //close pipes
  for(int i = 0; i < idx; ++i){
    close(fd_a[i*2]);
    close(fd_a[i*2+1]);
  }

  free(fd_a);
  fd_a = NULL;

  int fg_pid;
  job_struct js;

  if (!(holders[0].flags & BACKGROUND)) {
    // Not a background Job
    // TODO: Wait for all processes under the job to complete
    //IMPLEMENT_ME();
    while(is_empty_pid_queue (&process_queue) == false){
      fg_pid = pop_back_pid_queue(&process_queue);  
      waitpid(fg_pid, &status, 0);
    }
  }
  else {
    // A background job.
    bg_jobs_count++;
    // TODO: Push the new job to the job queue
    //IMPLEMENT_ME();
    int bg_pid;
    bg_pid = pop_front_pid_queue (&process_queue);

    js.job_id = bg_jobs_count;
    js.process_queue = &process_queue;

    push_back_job_queue (&bg_jobs_queue, js);
    // TODO: Once jobs are implemented, uncomment and fill the following line
    //char *cmd = strchr(get_command_string(), ' ');
    print_job_bg_start(js.job_id, bg_pid, "TODO");
  }
}
