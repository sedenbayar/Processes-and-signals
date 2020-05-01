#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <time.h>

#define NUM_CHILD 7

#define WITH_SIG

int CREATED_CHILDS = 0;
pid_t child_ids[NUM_CHILD];

//Keyboard interrupt handler.

#ifdef WITH_SIG
char interrupt_flag = 0;

//Print a message of the termination of child process.
void killed_child() {
	printf("child[%d]: Termination of the process.\n", getpid());
	exit(1);
}

void keyboard_interrupt() {
	printf("parent[%i]: The keyboard interrupt has been received.\n", getpid());
	interrupt_flag = 1;
}
#endif

//-------------------------------------------------------------------------------------
//The child process algorithm

void child_work(void) {
	printf("child[%d] I was spawned by parent PID %d!\n", getpid(), getppid());
	sleep(10);
	printf("child[%d] Terminating!\n", getpid());
	exit(0);
}

//------------------------------------------------------------------------------------
//Create childeren using fork

int create_children(int n) {
	for (int i = 0; i < n; ++i) {

		pid_t child_pid = fork(); // Forking new Process

		if (child_pid == -1) {

			printf("parent[%d] Error occured in fork().\n", getpid()); //creation of a child process was unsuccessful.

			//Killing all children.
			int j;
			for (j = 0; j < CREATED_CHILDS; ++j) {
				kill(child_ids[j], SIGTERM);
				printf("parent[%d] Killing all children.\n", getpid());
			}
			return -1;
		}

		//Copying all the children to a buffer on successfull creation.(Returned to parent)
		//Parent-----contains process ID of newly created child process
		if (child_pid > 0) {
        
			#ifdef WITH_SIG
			for(int j = 0; j < NSIG; ++j){ // NSIG the total number of signals defined
				signal(j, SIG_IGN); // ignore the signal
			}
			signal (SIGCHLD, SIG_DFL); //SIGCHLD - Termination of a child process & the parent will receive this signal
			
			//Set keyboard interrupt signal handler (symbol of this interrupt:  SIGINT)
			
			signal (SIGINT, keyboard_interrupt); //Ctrl C - global variable
			#endif

			child_ids[i] = child_pid;
			CREATED_CHILDS++;
			}

		//Child process because return value zero 
		if (child_pid == 0) {

		#ifdef WITH_SIG
			signal(SIGTERM, killed_child); // Calling the interrupt for killing the process.
			signal (SIGINT,SIG_DFL); //Default action associated with the signal occurs
		#endif

		child_work();
		
		}

		sleep(1);

		#ifdef WITH_SIG
		if (interrupt_flag == 1){
			printf("parent[%i]: Interrupt of the creation process!\n", getpid());
			kill(child_pid, SIGTERM); //Sends the signal sig to all processes whose process group ID is equal to the absolute value of pid/child_pid-2
		break;
		}
		#endif

	}

	printf("parent[%d] %d children spawned!\n", getpid(), CREATED_CHILDS);
	return n;
}

//-----------------------------------------------------------------------------------

int main(int argc, char ** argv) {

	printf("parent[%d] Alive!\n", getpid());

	int spawned_children = create_children(NUM_CHILD); //Create childeren
    
  
	if (CREATED_CHILDS == NUM_CHILD) //Check all processes are created
	printf("parent[%d]: All processes have been created.\n", (int) getpid());

	int child_status;
	pid_t w[CREATED_CHILDS];
	int exit_code[CREATED_CHILDS];

	int i;
	for (i = 0; i < CREATED_CHILDS; i++) {
		w[i] = wait(&child_status); //Blocks the calling process until one of its child processes exits

		if(w[i] == -1)
			break;
		else{
    		if(WIFEXITED (child_status)) //Exit status
    			exit_code[i] = WEXITSTATUS(child_status); //Returns the exit status of the child. This macro should be employed only if WIFEXITED returned true.
        	}
	}

	printf("parent[%d]: Terminated %d processes.\n", (int) getpid(), CREATED_CHILDS); //Termination

	#ifdef WITH_SIG
  		for(int j=0; j<NSIG; j++) //The old service handers of all signals should be restored
			signal(j, SIG_DFL);
  	#endif
	printf("parent[%d]: Parent process have been completed.\n", (int) getpid());
	printf("parent[%d] Terminating with exit code 0.\n", getpid());

	return 0;
}
