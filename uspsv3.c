#include <signal.h>
#include <stdlib.h>
#include "p1fxns.c"
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>

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

/*    set an alarm timer. */
void set_alarm_timer(int sec,int ms)
{
    //signal(SIGALRM,sig_timer);        /*subscribe to alarm signal*/

    struct itimerval new_value;
    new_value.it_interval.tv_sec = sec;    /* set the intervals */
    new_value.it_interval.tv_usec = ms;
    new_value.it_value.tv_sec = sec;
    new_value.it_value.tv_usec = ms;

    setitimer(ITIMER_REAL,&new_value,0);    /* Start the timer.*/

    printf("Inside set timer%d\n",getpid());
}

/*    timer signal handler. */
void sig_timer(int signal)
{
    printf("Stopping Self: %d\n",getpid());    
    fflush(stdout);
    kill(getpid(),SIGSTOP);            /* Stop this program */
    printf("Restarted: %d\n",getpid());    /* someone started us. */
    fflush(stdout);
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


pid_t fork_and_wait(struct Node *ll){
	char *cmd = ll->cmd;
	char **args = ll->args;
	pid_t returnValue = fork();
	//printf("AM I forking\n");

	if(returnValue == 0){
		while(run==0){//waiting for run to be set to 1 by sigusr1
			do_nanosleep(1);
		}
		execvp(cmd, args);
		//exit(1);
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

void initilize(struct Node* firstll){//might ned to subscribe in function too
	int exitcode = 0;
	struct Node* templl = firstll;
	while(templl!=NULL){
		fork_and_wait(templl);
		templl = templl->next;
	}
	templl = firstll;
	while(templl!=NULL){
		int exitcode = 0;
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
}

int main(int argc, char *argv[]){
	int filedesc;
	if (argc == 2){
        filedesc = open(argv[1], O_RDONLY);
    }
    else {
    	filedesc = 0;
    }
    signal(SIGALRM, sig_timer);   //alarm
	signal(SIGUSR1, sigusr1); //usr1
	signal(SIGCHLD, sig_function); // sigchld
	signal(SIGCONT, sig_function); // cont
	signal(SIGSTOP, sigstop); //stop

	//read in
	int terminator = 1;
	char buf[1024];
	char *token;
	struct Node* templl;
	struct Node* firsttodo;
	struct Node* lasttodo;
	struct Node* processing;
	struct Node* lastdone;
	struct Node* firstdone;
	char bs[50];
	int count, lines;
	lines = 0;
	int strlen;
	int x;
	char buf2[1024];
	terminator = p1getline(filedesc, buf, 1024);
	struct Node* newll;
	while(terminator){
		count = 0;
		x = p1getword(buf, x, buf2);
		newll = LList_create(buf2);
		x = 0;
		while(x!=-1){
			x = p1getword(buf, x, newll->args[count]);
			count++;
		}
		strlen = p1strlen(newll->cmd);
		newll->cmd[strlen] = '\0';
		if(lines>0){
			templl->next = newll;
			templl = templl->next;
		}
		else{
			firsttodo = newll;
		}
		templl=newll;
		terminator = p1getline(filedesc, buf, 300);
		lines++;
	}
	int exitcode = 0;
	initilize(firsttodo);
	templl = firsttodo;
	while(templl!=NULL){
		if (templl->next = NULL){
			templl = lasttodo;
		}
		templl = templl->next;
	}
	templl = firsttodo;
	set_alarm_timer(0,250);
	while (firsttodo!=NULL){
		kill(SIGCONT, templl->ID);
		kill(SIGALRM, templl->ID);
		exitcode = get_child_exit_code_if_exited(templl->ID);
		if (exitcode == 2){
			if (lastdone==NULL){
				firstdone = templl;
				lastdone = templl;
			}
			else{
				lastdone->next = templl;
				lastdone = lastdone->next;
			}
		}
		else{
			lasttodo->next = templl;
			lasttodo = lasttodo->next;
		}
		templl = firsttodo;
		firsttodo = firsttodo->next;
	}
	return 0;
}
