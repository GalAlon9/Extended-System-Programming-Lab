#include "LineParser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>



#define BUFF_SIZE 2048
int debug = 0;




int is_special(cmdLine *pCmdLine){


    if (strcmp(pCmdLine->arguments[0], "quit") == 0)
    {
        exit(0);
    }


    else if (strcmp(pCmdLine->arguments[0], "cd") == 0){
        
        if(chdir(pCmdLine->arguments[1]) == -1){

            perror("error changing directory");
        }

        return 1;

    }

    return 0;

}


void displayPrompt(){

    char cwd[PATH_MAX];
    getcwd(cwd, PATH_MAX);
    printf("%s$ ", cwd);
    
}



void pipe_cmd(cmdLine *pCmdLine){
    int p[2];

    if (pipe(p) < 0){

        perror("pipe error");
        exit(EXIT_FAILURE);

    }
    

    int pid1 = fork();
    if (pid1 == 0){
        
        dup2(p[1],1);
        close(p[1]);
        

        int err = execvp(pCmdLine->arguments[0], pCmdLine->arguments);
        if (err == -1) {
            perror("Error");
            _exit(1);
        }


    } 
    else{
    
        close(p[1]);
        int pid2 = fork();

        if (pid2 == 0){

            
            dup2(p[0],0);
            close(p[0]);
            

            int err = execvp(pCmdLine->next->arguments[0], pCmdLine->next->arguments);
            if (err == -1) {
                perror("Error");
                _exit(1);
            }

        } 
        else{

            
            close(p[0]);
           
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
            

        }

    }
           
}

void execute(cmdLine *pCmdLine){

    
    int in = 0; 
    int out = 1; 


    if (is_special(pCmdLine)){
        return;
    }
    
    int child_pid = fork();


    if(!child_pid){

        if(pCmdLine->inputRedirect){   

            in = open(pCmdLine->inputRedirect, O_RDONLY,0777);
            dup2(in, 0);
            close(in);
            if(in == -1){

                perror("error opening input file");
                _exit(1);

            }
            
        }



        if(pCmdLine->outputRedirect){

            out = open(pCmdLine->outputRedirect, O_WRONLY | O_CREAT | O_TRUNC, 0777);
            dup2(out, 1);
            close(out);
            if(out == -1){

                perror("error when opening output file");
                _exit(1);

            }

        }


        if(pCmdLine->next != NULL){

            pipe_cmd(pCmdLine);
        }


        else if(execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1){

            perror("error executing command");
            _exit(1);

        }
    }
    if (debug){

        fprintf(stderr, "PID: %d\n", child_pid);
        fprintf(stderr, "Executing command: %s\n", pCmdLine->arguments[0]);

    }


    if(pCmdLine->blocking){
        waitpid(child_pid, NULL, 0);
    }

}





int main(int argc, char **argv){

    char buff[BUFF_SIZE];

    if (argc >= 2 && strcmp(argv[1], "-d") == 0){

        debug = 1;
        printf("Debug mode is on\n");

    }


    while (1){

        displayPrompt();
        fgets(buff, BUFF_SIZE, stdin);
        cmdLine *parsedLine = parseCmdLines(buff);
        execute(parsedLine);
        freeCmdLines(parsedLine);


    }

    return 0;

}