/*
    Name: Safal Lamsal
    ID: 1001199093
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

struct Queue{
    int start, end, size, capacity;
    char* arr;
};

struct Queue* newQueue(int capacity) {
    struct Queue* queue = (struct Queue*)malloc(sizeof(struct Queue));
    queue->capacity = capacity;
    queue->start = 0;
    queue->size = 0;
    queue->end = capacity-1;
    queue->arr = (char*)malloc(queue->capacity*sizeof(char*));
    return queue;
}

int isEmpty(struct Queue* q) {
    return q->size == 0;
}

#define WHITESPACE " \t\n" //delimiters

#define MAX_COMMAND_SIZE 255 // max command size

#define MAX_NUM_ARGUMENTS 5  //max of 5 arguments

pid_t childpid = -5; //use in signal handling

//handles signals,  takes signal number
void sigHandler(int signum) {
    if(childpid > 0) {
        kill(childpid, signum);
        childpid = -5;
    }
}

//handles cd command , takes in parameters of the call
int cdhandle(char *args[]) {
    if(args[1] == NULL) {
        chdir(getenv("HOME"));
        return 1;
    }
/*
    if(strcmp(args[1], "..") == 0) {
        // go back a directory TODO
        char cwd[1024];
        printf("%s\n", getcwd(cwd, 1024));
        int i = 0;
        while(cwd[i]!='\0') {
            i++;
        }
        for(; i>0; i--) {
            if(cwd[i] == '/') {
                cwd[i] = '\0';
                break;
            }
        }
        chdir(cwd);
        return 1;
    }*/ else {
        if(chdir(args[1]) == -1) {
            printf("Invalid directory path.\n");
            return 1;
        }
    }

    return 0;
}

int main () {

    signal(SIGTSTP, sigHandler);
    signal(SIGINT, sigHandler);

    char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
    int pids[10]; // for showpids
    int pc = 0, pi = 0; //pids count, pids index

    while (1) {
        printf("msh> ");

        while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) ); //taking user input

        char *token[MAX_NUM_ARGUMENTS];

        int token_count = 0;

        char *arg_ptr;
        char *working_str = strdup( cmd_str );

        char *working_root = working_str;

        while(((arg_ptr=strsep(&working_str, WHITESPACE))!=NULL)&&(token_count<MAX_NUM_ARGUMENTS)){
            token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );

            if( strlen( token[token_count] ) == 0 ) {
                token[token_count] = NULL;
            }
                token_count++;
        }

        // blank input case
        if(token[0] == NULL) continue;

        //debug for input
        for (int i = 0; i < token_count-1; i++) {
            printf("%s\n", token[i]);
        }

        // termination cases
        if (strcmp(token[0], "exit") == 0 || strcmp(token[0], "quit") == 0) exit(0);

        // cd case
        if(strcmp(token[0], "cd") == 0) {
            cdhandle(token);
            continue;
        }

        //showpids case
        if(strcmp(token[0], "showpids") == 0) {
            //print out vector
            int i;
            for(i = 0; i < pc+1; i++) {
                if(pc>=10) break;
                printf("%d: %d\n", i, pids[i]);
            }
            continue;
        }

        if (fork() != 0) {
            //parent
            wait(NULL);
        } else {
            //exec right here
            childpid = getpid();
            if (pi < 10) {
                pids[pi] = childpid;
                pi++; pc++;
            } else {
                pids[0] = childpid;
                pi = 1;
            }

            if (execvp(token[0], token) == -1) {
                printf("%s: Commnad not found.\n\n", token[0]);
            }

            exit(0);
        }

        free( working_root );
    }

    return 0;
}
