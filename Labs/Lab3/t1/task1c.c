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
    //add new link to the end of the list 
    // link *newLink = (link *)malloc(sizeof(link));
    // link *start = virus_list;
    // newLink->vir = data;
    // newLink->nextVirus = NULL;
    // while(virus_list->nextVirus != NULL){
    //     virus_list = virus_list->nextVirus;
    // }
    // virus_list->nextVirus = newLink;
    // return start;
    //add new link to the start of the list
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
    
    // link *start = NULL;
    // virus *v;
    // int i =4;
    
    // //get the size of the file
    // int fileSize = 0;
    // fseek(file, 0, SEEK_END);
    // fileSize = ftell(file);
    // rewind(file);
    // //throw away the first 4 bytes
    // fseek(file, 4, SEEK_SET);
    // //read the file
    // while(i < fileSize){
    //     v = readVirus(file);
    //     start = list_append(start, v);
    //     i = i + 18 + v->SigSize;
    // }
    // fclose(file);
    // free(fileName);
    // return start;
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
    char *fileName = NULL;
    char read [BUFSIZ];
    FILE *file;
    printf("Enter the name of the file to search for viruses: ");
    fgets(read, BUFSIZ, stdin);
    read[strlen(read)-1] = '\0';
    fileName = (char *)malloc(strlen(read)+1);
    strcpy(fileName, read);
    file = fopen(fileName, "r");
    if(file == NULL){
        printf("Error opening file");
        exit(1);
    }
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
    free(fileName);
    free(buffer);
    return virus_list;
}


int main(int argc, char **argv){
    struct Menu menu[] = {{NULL,NULL},{"Load signatures", loadSignatures}, 
                          {"Print signatures", printSignatures},    
                          {"Detect viruses", detect},{NULL, NULL}};

    link *virus_list = NULL;
    int i;
    int input;
    char read[5];
    char *unused = "unused";
    while(1){
        printf("Please choose a function:\n");
        for(i=1; i<4;i++){
            printf("%d) %s\n", i, menu[i].name);
        }

        //get input option
        printf("option: ");
        scanf("%d", &input);
        fgetc(stdin);
        
        // if the input is within bounds then call the function
        if(input >=1 & input <= 3){
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