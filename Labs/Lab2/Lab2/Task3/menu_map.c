#include <stdlib.h>
#include <stdio.h>
#include <string.h>
 
char censor(char c) {
  if(c == '!')
    return '.';
  else
    return c;
}
 
char* map(char *array, int array_length, char (*f) (char)){
  char* mapped_array = (char*)(malloc(array_length*sizeof(char)));
  for(int i = 0; i < array_length; i++){
        mapped_array[i] = f(array[i]);
  }
  return mapped_array;
}


char my_get(char c){
    char temp = fgetc(stdin);
    return temp;
}

char cprt(char c){
    if(c >= 0x20 && c <= 0x7E){
        printf("%c \n", c);
    }
    else{
        printf(".\n");
    }
    return c;
}

char encrypt(char c){
    if(c >= 0x20 && c <= 0x7E){
        return c+3;
    }
    else{
        return c;
    }
}

char decrypt(char c){
     if(c >= 0x20 && c <= 0x7E){
        return c-3;
    }
    else{
        return c;
    }
}

char xprt(char c){
    printf("%x \n", c);
    return c;
}

char quit (char c){
    if(c == 'q'){
        exit(0);
    }
    else{
        return c;
    }
}

struct fun_desc {
  char *name;
  char (*fun)(char);
};


 
int main(int argc, char **argv){
    char * carray = (char*)malloc(5*sizeof(char));
    struct fun_desc menu[] = {{"Get string", my_get}, {"Print string", cprt}, {"Print hex", xprt}, {"Censor", censor}, {"Encrypt", encrypt}, {"Decrypt", decrypt}, {"Quit", quit}, {NULL, NULL}};
    int menu_size = sizeof(menu)/sizeof(menu[0]);
    int input;
    int i;
    char read[5];

    while(1){
        //print menu
        printf("Please choose a function: \n");
        for(i=0; i<menu_size-1; i++){
            printf("%d) %s \n", i, menu[i].name);
        }
        
        //get input option
        printf("option: ");
        fgets(read, 5, stdin);
        input = atoi(read);
        
        // if the input is within bounds then call the function
        if(input >=0 && input <= menu_size-1){
            printf("Within bounds \n");
            // char* mapped_array = map(carray, 5, menu[input].fun);
            // free(carray);
            carray = map(carray, 5, menu[input].fun);
            printf("DONE. \n \n");
        }
        
        //otherwise exit
        else{
            printf("Not within bounds \n");
            exit(0);
        }
    }
    // free(carray);
    return 0;
}
