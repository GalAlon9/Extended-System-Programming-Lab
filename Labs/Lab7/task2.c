//Your task: Write a short program called mypipeline which creates a pipeline of 2 child processes. Essentially, you will implement the shell call "ls -l | tail -n 2".

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

int debug = 0;

void displayPrompt()
{
    char cwd[PATH_MAX];
    getcwd(cwd, PATH_MAX);
    printf("%s$ ", cwd);
}

int main(int argc, char **argv){
    int child1, child2;
    int fd[2];
    char *args1[] = {"ls", "-l", 0};
    char *args2[] = {"tail", "-n", "2", 0};
    int status;

    for (int i = 0; i < argc; i++)
        if (strcmp(argv[i], "-d") == 0)
            debug = 1;
    // create pipe
    status = pipe(fd);
    if (status == -1){
        perror("pipe");
        exit(1);
    }

    if (debug) printf("%s (parent_process>forking…)\n", "");


    // fork child1
    child1 = fork();
    if (child1 == -1){
        perror("fork");
        exit(1);
    }
    if (debug) printf("%s (parent_process>created process with id: %d)\n", "", getpid());


    // child1 process
    if(!child1){
        if (debug) printf("%s (child1>redirecting stdout to the write end of the pipe…)\n", "");

        // redirect stdout to the write end of the pipe
        close(1);
        dup2(fd[1], 1);
        close(fd[1]);

        if (debug) printf("%s (child1>going to execute cmd: …)\n", "");
        // execute cmd: ls -l
        execvp(args1[0], args1);
        perror("execvp");
        exit(1);
    }
    else{
        if (debug) printf("%s (parent_process>closing the write end of the pipe…)\n", "");
        // close the write end of the pipe
        close(fd[1]);
        if(debug) printf("%s (parent_process>forking…)\n", "");
        // fork child2
        child2 = fork();
        if (child2 == -1){
            perror("fork");
            exit(1);
        }
        if (debug) printf("%s (parent_process>created process with id: %d)\n","",  getpid());

        // child2 process
        if(!child2){
            if (debug) printf("%s (child2>redirecting stdin to the read end of the pipe…)\n","");
            // redirect stdin to the read end of the pipe
            close(0);
            dup2(fd[0], 0);
            close(fd[0]);

            if(debug) printf("%s (child2>going to execute cmd: …)\n","");
            // execute cmd: tail -n 2
            execvp(args2[0], args2);
            perror("execvp");
            exit(1);
        }
        else{
            if (debug) printf("%s (parent_process>closing the read end of the pipe…)\n","");
            // close the read end of the pipe
            close(fd[0]);
            
            // wait for child processes to terminate
            if (debug) printf("%s (parent_process>waiting for child processes to terminate…)\n","");
            waitpid(child1, &status, 0);
            if (debug) printf("%s (parent_process>waiting for child processes to terminate…)\n","");
            waitpid(child2, &status, 0);
        }
    }
    return 0;
}