#include <signal.h>
#include "p1fxns.c"
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

volatile int run=0;
void sigusr1(int signal){//from lab
	run = 1;
}
struct Node{
	struct Node* next;
	char cmd[50];  //possibly change to dynamic
	char args[50][1024];
	pid_t ID;
};

/* ALL FROM LAB */
void do_nanosleep(int nseconds){
	struct timespec time, time2;
	time.tv_sec = 0;
	time.tv_nsec = nseconds;
	time2.tv_sec = 0;
	time2.tv_nsec =0;
	nanosleep( &time, &time2);
}
struct sigprinter{
	const char *signame;
	int signumber;
	const char *sigdescription;
};
struct sigprinter Signals[] =
{
{0,0,0},
{"SIGUSR1" , 10, " User defined signal 1"},
{"SIGALARM", 14, "Alarm clock"},
{"SIGCHLD", 17, "Child process has stopped or exited, changed"},
{"SIGCONT", 18, "continue executing, if stopped"},
{"SIGSTOP", 19, "Stop executing"},
{0,0,0}
};


int get_child_exit_code_if_exited(pid_t pid)
{
    
    int status;
    int result = waitpid(pid,&status,WNOHANG);

    if(result !=pid)
    {
        printf("%d\tProcess %d is running normally. \r\n",getpid(),pid);
        return 1;

    }
    if(WIFEXITED(status))
    {
        printf("%d\tProcess %d is exited normally. \r\n",getpid(),pid);
        int es = WEXITSTATUS(status);
               printf("Exit status was %d\n", es);
        return 2;
    }
    else if( WIFSIGNALED(status))
    {
        printf("%d\tProcess %d is exited by signal. \r\n",getpid(),pid);
        return 3;
    }
    else if(WIFSTOPPED(status))
    {
        printf("%d\tProcess %d is stopped. \r\n",getpid(),pid);
        return 4;
    }
    else if(WIFCONTINUED(status))
    {
        printf("%d\tProcess %d is continued. \r\n",getpid(),pid);
        return 5;
    }
    else
    {
        printf("%d\tProcess %d is still running. \r\n",getpid(),pid);
        return 6;
    }
}

/*end of from lab */


void sig_function(int signal){   // from lab
	struct sigprinter *ptr = & ( Signals[signal]);
}

void sigstop(int signal){
	printf("stopping...");
}

struct Node* LList_create(char* cmdx){
	struct Node *l = (struct Node *)malloc(sizeof(struct Node));
	if (l!=NULL){
		l->next = NULL;
		p1strcpy(l->cmd, cmdx);
		 //chjeck to make sure we don't need to strdup or something here
		l->ID;
		 //not sure
		return l;
	}
	else{
		return NULL;
	}
}

void wait_and_exec(char *cmd, char* argv[]){
	while(run==0){//waiting for run to be set to 1 by sigusr1
			do_nanosleep(1);
		}
	execvp(cmd, argv);
	printf("This should not be reached\n");
	exit(0);
}
pid_t fork_and_wait(struct Node *ll){
	char *cmd = ll->cmd;
	char **args = ll->args;
	pid_t returnValue = fork();
	//printf("AM I forking\n");

	if(returnValue == 0){
		wait_and_exec(cmd, args);
	}
	ll->ID = returnValue;//THIS LINE IS SEG FAULTING

	return returnValue;
}
void freeallNodes(struct Node* node){
	struct Node *prev;
	while(node!=NULL){
		
		prev = node;
		node = prev->next;
		free(prev);
	}
}

int main(int argc, char *argv[]){
	int filedesc;
	if (argc == 2){
        filedesc = open(argv[1], O_RDONLY);
    }
    else {
    	filedesc = 0;
    }

	signal(SIGUSR1, sigusr1); //usr1
	signal(SIGCHLD, sig_function); // sigchld
	signal(SIGCONT, sig_function); // cont
	signal(SIGSTOP, sigstop); //stop
	/*fromt project 1 */
	//int filedesc = open("testing.txt", O_RDONLY);
	int terminator = 1;
	char buf[1024];
	char *token;
	struct Node* templl;
	struct Node* firstll;
	char bs[50];
	int count, lines;
	lines = 0;
	terminator = p1getline(filedesc, buf, 1024);
	struct Node* newll;
	while(terminator){
		count = 0;
		token = strtok(buf, " ");
		newll = LList_create(token);
		token = strtok(NULL, " ");
		while(token!=NULL){
			p1strcpy(newll->args[count], token);
			token = strtok(NULL, " ");
			count++;

		}
		if(lines>0){
			templl->next = newll;
			templl = templl->next;
		}
		else{
			firstll = newll;
		}
		templl=newll;
		terminator = p1getline(filedesc, buf, 300);
		lines++;
	}
	int exitcode;
	templl = firstll;
	while(templl!=NULL){
		fork_and_wait(templl);
		templl = templl->next;
	}
	templl = firstll;
	while(templl!=NULL){
		exitcode = 0;
		kill(templl->ID, SIGUSR1);
		exitcode = get_child_exit_code_if_exited(templl->ID);
		/*
		while (exitcode!=1){
			exitcode = get_child_exit_code_if_exited(templl->ID);
		}
		*/
		templl = templl->next;
	}
	templl = firstll;
	while(templl!=NULL){
		exitcode = 0;
		kill(templl->ID, SIGSTOP);
		exitcode = get_child_exit_code_if_exited(templl->ID);
		/*
		while (exitcode!=4){
			printf("no\n");
			kill(templl->ID, SIGSTOP);
			do_nanosleep(100);
			exitcode = get_child_exit_code_if_exited(templl->ID);
		}
		*/

		templl = templl->next;
	}
	templl = firstll;
	while(templl!=NULL){
		exitcode = 0;
		exitcode = get_child_exit_code_if_exited(templl->ID);
		kill(templl->ID, SIGCONT);
		/*while (exitcode!=2){
			do_nanosleep(100);
			exitcode = get_child_exit_code_if_exited(templl->ID);
		}
		*/
		templl = templl->next;
	}
	freeallNodes(firstll);
	return 0;
}
	
	
