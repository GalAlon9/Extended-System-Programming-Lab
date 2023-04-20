#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef struct virus {
    unsigned short SigSize;
    char virusName[16];
    unsigned char* sig;
} virus;

typedef struct link link;

struct link {
    link *nextVirus;
    virus *vir;
};

virus* readVirus(FILE* file){
    virus *v = (virus *)malloc(sizeof(virus));
    if(fread(v, 1, 18 , file) != 18){
        free(v);
        return NULL;
    }
    else{
        v->sig = (unsigned char *)malloc(v->SigSize);
        if(fread(v->sig, 1, v->SigSize, file) != v->SigSize){
            free(v->sig);
            free(v);
            return NULL;
        }
        else{
            return v;
        }
    }
    
}

typedef struct Menu{
    char *name;
    link* (*fun)(link*, char*);
}Menu;


FILE *get_file(){
    char *fileName = NULL;
    char read [BUFSIZ];
    FILE *file;
    printf("Enter the name of the file to fix: ");
    fgets(read, BUFSIZ, stdin);
    read[strlen(read)-1] = '\0';
    fileName = (char *)malloc(strlen(read)+1);
    strcpy(fileName, read);
    file = fopen(fileName, "r+");
    return file;
}

void printVirus(virus* virus, FILE* output){
    fprintf(output, "Virus name: %s\n", virus->virusName);
    fprintf(output, "Virus size: %d\n", virus->SigSize);
    fprintf(output, "signature:\n");
    int i;
    for(i = 0; i < virus->SigSize; i++){
        fprintf(output, "%02X ", virus->sig[i]);
    }
    printf("\n");
}

void list_print(link *virus_list, FILE* output){
    /* Print the data of every link in list to the given stream. Each item followed by a newline character. */
    link *current = virus_list;
    while(current != NULL){
        printVirus(current->vir, output);
        current = current->nextVirus;
    }
}

link* list_append(link* virus_list, virus* data){ 
    
    link *newLink = (link *)malloc(sizeof(link));
    newLink->vir = data;
    newLink->nextVirus = virus_list;
    return newLink;

}

void list_free(link *virus_list){ 
    /* Free the memory allocated by the list. */
    link *current = virus_list;
    while(current != NULL){
        link *temp = current;
        current = current->nextVirus;
        free(temp->vir->sig);
        free(temp->vir);
        free(temp);
    }
}

link* loadSignatures(link *virus_list, char* unused){
    
  
    char *fileName = NULL;
    char buffer[BUFSIZ];
    FILE *file;
    printf("Please enter the file name: ");
    fgets(buffer, BUFSIZ, stdin);
    buffer[strlen(buffer)-1] = '\0';
    fileName = (char *)malloc(strlen(buffer)+1);
    strcpy(fileName, buffer);
    file = fopen(fileName, "r");
    
    int bytes = 4;
    int fileSize;
    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    rewind(file);
    //reamove the first 4 bytes
    fseek(file, 4, SEEK_SET);

    while(bytes < fileSize){
        link *newLink = (link *)malloc(sizeof(link));
        virus *v = (virus *)malloc(sizeof(virus));
        newLink->nextVirus = NULL;
        v = readVirus(file);
        newLink->vir = v;
        virus_list = list_append(virus_list, v);
        bytes = bytes + 18 + v->SigSize;
    }
    return virus_list;
}

link *printSignatures(link *virus_list, char* unused){
    FILE *output = stdout;
    list_print(virus_list, output);
    return virus_list;
}

void detect_virus(char *buffer, unsigned int size, link *virus_list){
    int i;
    virus *v;
    while(virus_list != NULL){
        v = virus_list->vir;
        for(i=0;i<size - v->SigSize;i++){
            if(memcmp(buffer+i, v->sig, v->SigSize) == 0){
                printf("Starting byte: %d\n", i);
                printf("Virus name: %s\n", v->virusName);
                printf("Virus size: %d\n", v->SigSize);
            }
        }
        virus_list = virus_list->nextVirus;
    }
}

link *detect(link *virus_list, char *unused){
    FILE *file = get_file();
    //get the size of the file
    int fileSize = 0;
    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    rewind(file);
    //detect 
    char *buffer = (char *)malloc(fileSize);
    fread(buffer, 1, fileSize, file);
    detect_virus(buffer, fileSize, virus_list);
    fclose(file);
    // free(fileName);
    free(buffer);
    return virus_list;
}

void kill_virus(FILE *file, unsigned short location, unsigned short virus_size){
    //move to the location of the virus
    fseek(file, location, SEEK_SET);
    //replace with NOPs
    for(int i=0;i<virus_size;i++){
        fputc(0x90, file);
    }
}

link *fix_file(link *unused_list, char *unused){
    // get the file to be fixed from user
    FILE *file = get_file();
    unsigned int  virus_size;
    unsigned int location;
    // get the size and location of the virus
    printf("enter the size and the location of the virus: \n");
    scanf("%d %d", &virus_size, &location);
    //kill the virus
    kill_virus(file, location, virus_size);
    fclose(file);
    return unused_list;
}




int main(int argc, char **argv){
    struct Menu menu[] = {{NULL,NULL},{"Load signatures", loadSignatures}, 
                          {"Print signatures", printSignatures},    
                          {"Detect viruses", detect},
                          {"Fix file",fix_file},
                          {NULL, NULL}};

    link *virus_list = NULL;
    int i;
    int input;
    char read[5];
    char *unused = "unused";
    while(1){
        printf("Please choose a function:\n");
        for(i=1; i<5;i++){
            printf("%d) %s\n", i, menu[i].name);
        }

        //get input option
        printf("option: ");
        scanf("%d", &input);
        fgetc(stdin);
        
        // if the input is within bounds then call the function
        if(input >=1 & input <= 4){
            virus_list = menu[input].fun(virus_list,unused);
            printf("Within bounds \n");
            printf("DONE. \n \n");
        }
        //otherwise exit
        else{
            printf("Not within bounds \n");
            exit(0);
        }

    }
    
}