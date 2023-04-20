#include "LineParser.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/stat.h>



#define MAX_HISTORY 4


void free_hist(char *history){
    int i=0;
    while(i<MAX_HISTORY){
        free(history[i]);
        i++;
    }
}

void add_to_hist(char *hist, char *cmd){
    int i = MAX_HISTORY-1;
    hist[0] = cmd;
    while(i>0){
        hist[i] = hist[i-1];
        i--;
    }
}

void displayPrompt()
{
    char cwd[PATH_MAX];
    getcwd(cwd, PATH_MAX);
    printf("%s$ ", cwd);
}

int catch_command(cmdLine* command, char **history){  
    int ret = 0;
    if(strcmp(command->arguments[0], "history") == 0){

        ret = 1;
        for(int i = 0; i < MAX_HISTORY; i++){
            if(history[i] != NULL){
                printf("%d: %s", i + 1, history[i]);
            }
        }
    }

    if(strcmp(command->arguments[0], "quit") == 0){
        free_hist(history);
        exit(0);
    }


    if(strcmp(command->arguments[0], "cd") == 0){
        if(chdir(command->arguments[1]) == -1){
            perror("cd error \n");
        }
        ret = 1;
    }
    
    if(command->blocking){
        int loc = atoi(command->arguments[0] + 1);
        cmdLine *newCmd = parseCmdLines(history[loc - 1]);
        execute(newCmd, 0, history);
        char *temp[strlen(history[loc - 1])];
        strcpy(temp, history[loc - 1]);
        add_to_hist(history, temp);
        ret = 1;

    }


    else if(command->arguments[0][0] == '!' && command->arguments[0][1] == '!'){

        cmdLine *newCmd = parseCmdLines(history[0]);
        execute(newCmd, 0, history);
        int len = strlen(history[0]);
        char *temp [len];
        strcpy(temp, history[0]);
        add_to_hist(history, temp);
        ret = 1;
    }

    return ret;
}




void execute(cmdLine* pCmdLine, int debug, char **history){

    int out;
    int in;
    int fd[2];
    pid_t pid1, pid2;

    if(catch_command(pCmdLine, history)){
        return;
    }


    
    if (pCmdLine->next != NULL){ 
        
        int status = pipe(fd); 
        if (status == -1){
            perror("Pipe error");
            exit(1);
        }

        if((pid1 = fork()) == 0){ 

            if(pCmdLine->inputRedirect){ 

                in = open(pCmdLine->inputRedirect, O_RDONLY);
                int temp = dup2(in, 0);
                if (temp < 0){
                    perror("error duping input file");
                    exit(1);
                }
                if(in < 0){
                    perror("error opening input file");
                    exit(1);
                }
                
            }
                
            close(1);
            dup2(fd[1], 1);
            close(fd[1]);

            if(execvp(pCmdLine->arguments[0], pCmdLine->arguments) < 0){
                perror("Cannot execute command");
                freeCmdLines(pCmdLine);
                _exit(1); 
            }

            close(in);
            exit(0);


        } 
        else { 
            if (pid1 > 0){

                close(fd[1]);
                if((pid2 = fork()) == 0){
                    if(pCmdLine->next->outputRedirect){
                        out = open(pCmdLine->next->outputRedirect, O_WRONLY | O_CREAT);
                        if (dup2(out, 1) < 0){

                            perror("error duping output file");
                            exit(1);

                        }
                        if(out < 0){

                            perror("error opening output file");
                            exit(1);

                        }
                    }
                    close(0);
                    dup2(fd[0], 0);
                    close(fd[0]);

                    if(execvp(pCmdLine->next->arguments[0], pCmdLine->next->arguments)<0){
                        perror("illegal command");
                        freeCmdLines(pCmdLine);
                        _exit(1); 
                    }
                    close(out);
                    exit(0);
                }

                else{
                    close(fd[0]);
                    waitpid(pid1,NULL,0);
                    waitpid(pid2,NULL,0);

                }
            }
        }
    }

    else if((pid1 = fork()) == 0){ 
        if(pCmdLine->inputRedirect){
            in = open(pCmdLine->inputRedirect, O_RDONLY);
            if(in < 0 || dup2(in, 0) < 0){

                perror("error duping input file");
                exit(EXIT_FAILURE);

            }
        }

        if(pCmdLine->outputRedirect){
            out = open(pCmdLine->outputRedirect, O_WRONLY | O_CREAT);
            if(out < 0){
                perror("eror in opening output file");
                exit(EXIT_FAILURE);
            }

            if (dup2(out, 1) < 0){

                perror("error while duping output file");
                exit(EXIT_FAILURE);

            }
        }
        if(debug){

            fprintf(stderr, "PID: %d\nExecuting command: %s\n", getpid(), pCmdLine->arguments[0]);
        }

        if(execvp(pCmdLine->arguments[0], pCmdLine->arguments)<0){

            perror("error with execute command");
            freeCmdLines(pCmdLine);
            _exit(EXIT_FAILURE);  

        }

        close(out);
        close(in);

        _exit(0);
    }
    else {
        if (pid1){

            if(debug){
                fprintf(stderr, "PID: %d\nExecuting command: %s\n", pid1, pCmdLine->arguments[0]);
            }
            
            if(pCmdLine->blocking){
                waitpid(pid1, NULL ,0);
            }

        }
    }  
    
}



int debug = 0;

int main(int argc, char **argv){

    cmdLine *pCmdLine ;
    char buff[2048] ;
    for (int i = 0 ; i < argc ; i++){
        if (strcmp("-d", argv[i]) == 0){
            debug = 1;
        }
    }

    char * history [MAX_HISTORY];
    for (int i = 0 ; i < MAX_HISTORY ; i++){
        history[i] = NULL;
    }

    while (1){
        displayPrompt();
        fgets(buff, 2048, stdin);
        if (buff[0] != '!'){
            char *cmdcopy = malloc(strlen(buff) + 1);
            strcpy(cmdcopy, buff);

            if (buff[0] != '\n'){
                add_to_hist(history, cmdcopy);
            }
        }



        pCmdLine = parseCmdLines(buff);
        if (pCmdLine == NULL){
            break;
        }
        if (debug){
            fprintf(stderr, "parent PID: %d\n", getpid());
        }
        execute(pCmdLine, debug, history);
        freeCmdLines(pCmdLine);

    }
    return 0;
}
