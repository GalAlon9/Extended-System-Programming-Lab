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

void displayPrompt()
{
    char cwd[PATH_MAX];
    getcwd(cwd, PATH_MAX);
    printf("%s$ ", cwd);
}

int isSpeacial(cmdLine *pCmdLine)
{
    if (strcmp(pCmdLine->arguments[0], "quit") == 0)
    {
        exit(0);
    }
    else if (strcmp(pCmdLine->arguments[0], "cd") == 0)
    {
        if(chdir(pCmdLine->arguments[1]) == -1)
        {
            perror("Error changing directory");
        }
        return 1;
    }
    return 0;
}

void execute(cmdLine *pCmdLine)
{
    int input = 0; //stdin
    int output = 1; //stdout
    if (isSpeacial(pCmdLine))
    {
        return;
    }
    
    int child_pid = fork();
    if(!child_pid)
    {
        if(pCmdLine->inputRedirect)
        {   
            close(input);
            input = open(pCmdLine->inputRedirect, O_RDONLY);
            if(input == -1)
            {
                perror("Error opening input file");
                _exit(1);
            }
            
        }
        if(pCmdLine->outputRedirect)
        {
            close(output);
            output = open(pCmdLine->outputRedirect, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if(output == -1)
            {
                perror("Error opening output file");
                _exit(1);
            }
        }
        if(execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1)
        {
            perror("Error executing command");
            _exit(1);
        }
    }
    if (debug)
    {
        fprintf(stderr, "PID: %d\n", child_pid);
        fprintf(stderr, "Executing command: %s\n", pCmdLine->arguments[0]);
    }
    if(pCmdLine->blocking)
    {
        waitpid(child_pid, NULL, 0);
    }



}



int main(int argc, char **argv)
{
    char buff[BUFF_SIZE];
    if (argc > 1 && strcmp(argv[1], "-d") == 0)
    {
        debug = 1;
        printf("Debug mode is on\n");
    }


    while (1)
    {
        displayPrompt();
        fgets(buff, BUFF_SIZE, stdin);
        cmdLine *parsedLine = parseCmdLines(buff);
        execute(parsedLine);
        freeCmdLines(parsedLine);
    }
    return 0;
}