#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <poll.h>
#include <ctype.h>

pid_t pid; //process id global variable
pid_t child; //child process global variable

int is_Digit(char *str){ //function to check if a string is a digit
    for (int i=0; i<strlen(str); i++){
        if (!isdigit(str[i])){
            return 0;
        }
    }    return 1;
}

int main(int argc, char* argv[]){ //argc = number of arguments (1 default), argv = arguments
char command [100]; //command buffer to read input

int task; //the number given to the child to process to decrement
int N; //ammount of children parent will create
N=atoi(argv[1]); //argument to intiger function

int childpids[N]; //array to store the child pids
int child_id; //shoutout to pjf for the idea
int task_to; //children which has gotten the task

//dynamic allocation for pipes
int (parent_to_child)[N][2]; //pipe parent to child
int (child_to_parent)[N][2]; //receive pipe descriptor  

//start to check for Command Line arguments
int default_mode = 0; //default mode = round robin (0) or random (1)
if (argc==2 && N>=1){
    printf("Default mode: Round Robin\n");
}else if (argc==3 && N>=1 && (strcmp(argv[2],"--round-robin")==0)){
    printf("Default mode: Round Robin\n");
}else if (argc==3 && N>=1 && (strcmp(argv[2],"--random")==0)){
    default_mode = 1;
    printf("Default mode: Random\n");
}else{
    printf("Usage: ask3 <nChildren> [--random] [--round-robin]\n");
    return 0;
}

for (int i=0; i<N; i++){ //create pipes for n proccesses
if (pipe(parent_to_child[i]) == -1 || pipe(child_to_parent[i]) == -1){ //create pipes and check for errors simultaneously
    perror("PIPE CREATION FAILED");
    exit(EXIT_FAILURE); }
}

for (int i=0; i<N; i++){
    if ((child=fork())==-1){
        perror("FORK FAILED");
        exit(EXIT_FAILURE);
    }else if (child==0){
        //child code
        //printing created child i with pid
        printf("Child %d created with pid: %d and ppid: %d\n",i,getpid(),getppid());
        exit(0);
    }else {
        wait(NULL);
        //parent code
        childpids[i] = child; //store the child pids
        //print child pids
        printf("Child %d pid: %d\n",i,childpids[i]);
    }
}

//create poll structure
struct pollfd fds[N+1]; //N child_to_parent pipes + 1 for stdin (CLI)
//for child to parent pipes
for (int i=0; i<N; i++){
    fds[i].fd = child_to_parent[i][0]; //read end of pipe
    fds[i].events = POLLIN; //check for data to read
} 
//for stdin
fds[N].fd = STDIN_FILENO; //stdin
fds[N].events = POLLIN; //check for data to read

/*debugging purposes
for (int i=0; i<N; i++){
    printf("Child %d pid: %d\n",i,childpids[i]);
}
debugging purposes */

int quit = 0; //quit flag
    while (!quit){
        int pollcount = poll(fds, N+1, -1); //poll for data to read
        if (pollcount<0){
            perror("poll failed"); //if error than exit
            exit(EXIT_FAILURE);
        }
        if (fds[N].revents & POLLIN){ //if there is data to read from stdin
        scanf("%s",command); //read the command
        printf("Command:%s\n",command); //print the command
        }
        if(strcmp(command,"exit")==0){ //if the command is exit
            quit = 1; //set the quit flag to 1
            //terminate all children processes & parent
            break; //break the loop
        }else if (strcmp(command,"help")==0){
            printf("Type a number to send job to a child!\n");
        }else if (is_Digit(command)==1){ //check if it is an integer , then distribute number to child 
            task = atoi(command);  
            if(default_mode==0){ //round-robin
            child_id = (child_id + 1) % N; 
            task_to = child_id; 
            }
            if(default_mode==1){ //random
            child_id = rand() % (N + 1); 
            task_to = child_id;
            }
        }else {
            printf("Type a number to send job to a child!\n");
        }
    }


return 0;
}