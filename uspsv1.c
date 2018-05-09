#include "p1fxns.c"
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>



volatile int run = 0;
volatile int child_exited= 0;

struct Node{
	struct Node* next;
	char cmd[50];  //possibly change to dynamic
	char args[50][1024];
	pid_t id;
};



struct Node* LList_create(char* cmdx){
	struct Node *l = (struct Node *)malloc(sizeof(struct Node));
	if (l!=NULL){
		l->next = NULL;
		p1strcpy(l->cmd, cmdx);
		 //chjeck to make sure we don't need to strdup or something here
		l->id;
		 //not sure
		return l;
	}
	else{
		return NULL;
	}
}

void freeallNodes(struct Node* node){
	struct Node *prev;
	while(node!=NULL){
		
		prev = node;
		node = prev->next;
		free(prev);
	}
}

struct Node* firstll;

/* FROM LAB */
void do_nanosleep(int nseconds){
	struct timespec time, time2;
	time.tv_sec = 0;
	time.tv_nsec = nseconds;
	time2.tv_sec = 0;
	time2.tv_nsec =0;
	nanosleep( &time, &time2);
}

void forkandrun(struct Node* myll){
		myll->id = fork();
		if(myll->id ==  0){

			//prepare argument structure;
			execvp(myll->cmd, myll->args);

			exit(1);
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
    int x;
	int terminator = 1;
	int strlen;
	char buf[1024];
	char buf2[1024];
	char *token;
	struct Node* templl;
	char bs[50];
	//templl = LList_create(bs);
	int count, lines;
	
	terminator = p1getline(filedesc, buf, 1024);
	struct Node* newll;
	lines =0;
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
			firstll = newll;
		}
		templl = newll;
		terminator = p1getline(filedesc, buf, 300);
		lines++;

	}
	templl = firstll;
	while(templl!=NULL){

		forkandrun(templl);
		templl = templl->next;

	}

	templl = firstll;
	while(templl!=NULL){
		waitpid(templl->id, NULL, WNOHANG);
		templl = templl->next;
	}
	templl = firstll;
	freeallNodes(firstll);
	return 0;
}