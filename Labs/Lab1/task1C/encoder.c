//function that gets FILE* and return it upper case
#include <stdio.h>
#include <string.h>
#define BUFFER_SIZE (128)

int main(int argc, char **argv)
{
    FILE * input = stdin;
    FILE * output = stdout;

    char curChar;
    int i;
    int j = 0;

    int debug = 0;
    int extended = 0;
    int normal = 1;


    char * code;
    char value;
    
    //catch what kind of mode we are in from the arguments
    for(i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "-D") == 0)//debug mode
        {
            debug = 1;
            normal = 0;
            printf("-D\n");
        }
        else if(strncmp(argv[i],"+e",2)==0){//extended mode +
            normal=0;
            extended = 1;
            code = argv[i] +2;

        }
        else if(strncmp(argv[i],"-e",2)==0){//extended mode -
            normal=0;
            extended = -1;
            code = argv[i] +2;
        }
        else if(strncmp(argv[i],"-i",2)==0)//input file - task 2a
        {
            input = fopen(argv[i]+2, "r");
            if(input == NULL)
            {
                printf("Error opening input file");
                return 1;
            }
        }
        else if(strncmp(argv[i],"-o",2)==0)//output file - task 2b
        {
            output = fopen(argv[i]+2, "w");
            if(output == NULL)
            {
                printf("Error opening output file");
                return 1;
            }
        }
        else
        {
            printf("Error: invalid argument");
            return 1;
        }
        
    }


    do{ //this is the loop that reads the file
        curChar = fgetc(input);
        if(curChar == EOF)//if we reached the end of the file
            break;
        if(normal ==1){//normal mode
            if(curChar >= 'a' && curChar <= 'z')
                curChar = curChar - 'a' + 'A';
            fputc(curChar, output);
        }
        else if(debug == 1) {//debug mode
            if(curChar != '\n'){
                fprintf(stderr, "%x ", curChar);
                if(curChar >= 'a' && curChar <= 'z')
                    curChar = curChar - 'a' + 'A';
                fprintf(stderr, "%x\n", curChar);
                fputc(curChar, output);
            }
        }
        else if (extended == 1){//extended mode +
            if(curChar != '\n'){
                value = code[j];
                j = j+1;
                curChar = curChar + value - 48;
                fputc(curChar, output);
            }
            else if(curChar == '\n'){
                j = 0;
            }
        }

        else if (extended == -1){//extended mode -
            if(curChar != '\n'){
                value = code[j];
                j = j+1;
                curChar = curChar - value + 48;
                fputc(curChar, output);
            }
            else if(curChar == '\n'){
                j = 0;
            }
        }
        
    }while(1);
}

