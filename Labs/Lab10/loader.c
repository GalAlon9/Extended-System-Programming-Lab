#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <elf.h>


int curr_fd = -1;
struct stat file_stat;
void *file_map; //pointer to the start of the mapped file
Elf32_Ehdr *elf_header;
int load_file(char *file_name);
void iterate_over_phdr(char *file_name);
int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *,int), int arg);
void print_phdr(Elf32_Phdr *phdr, int arg);
void print_all_info(Elf32_Phdr *phdr, int arg);
char *get_type(int type);
char *get_flags(int flags);
char *get_align(int align);
void print_all_info_extedended(Elf32_Phdr *phdr,int arg);
void load_phdr(Elf32_Phdr *phdr, int fd);

extern int startup();


int load_file(char *file_name)
{
    int fd = open(file_name, O_RDWR);
    if (fd == -1)
    {
        perror("Error opening file");
        return -1;
    }
    if(fstat(fd, &file_stat) < 0)
    {
        perror("Error fstat");
        return -1;
    }
    file_map = mmap(0, file_stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (file_map == MAP_FAILED)
    {
        perror("Error mapping file");
        return -1;
    }
    //close any currently open file
    if (curr_fd != -1)
    {
        close(curr_fd);
    }
    curr_fd = fd;
    return fd;
}


void iterate_over_phdr(char *file_name){
    int fd = load_file(file_name);
    if (fd == -1)
    {
        return;
    }
    elf_header = (Elf32_Ehdr *) file_map;
    //check if the file is an ELF file from the first 4 bytes or not 32bit
    if (elf_header->e_ident[0] != 0x7f || elf_header->e_ident[1] != 'E' || elf_header->e_ident[2] != 'L' || elf_header->e_ident[3] != 'F' || elf_header->e_ident[4] != ELFCLASS32)
    {
        fprintf(stderr, "Error: Not a 32bit ELF file\n");
        munmap(file_map, file_stat.st_size);
        close(fd);
        free(file_name);
        return;
    }
    //iterate over program headers, for each one call foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *,int), int arg);
    // print : "Program header number i at address x" for each program header
    Elf32_Phdr *phdr = (Elf32_Phdr *) (file_map + elf_header->e_phoff);
    for (int i = 0; i < elf_header->e_phnum; i++)
    {
        phdr++;

    }

  
}

int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *,int), int arg){
    //apply func to each program header
    elf_header = (Elf32_Ehdr *) file_map;
    for (int i = 0; i < elf_header->e_phnum; i++)
    {
        func((Elf32_Phdr*)map_start, i);
        map_start += elf_header->e_phentsize;
    }
    return 0;
}

//task 0
void print_phdr(Elf32_Phdr *phdr, int arg){
    fprintf(stdout, "Program header number %d at address %p\n", arg, phdr);
}

//task 1a
void print_all_info(Elf32_Phdr *phdr, int arg){
    /*
    print all the information which resides in the corresponding Elf32_Phdr structure.

    The output should look similar to readelf -l:

    Type Offset VirtAddr PhysAddr FileSiz MemSiz Flg Align
    PHDR 0x000034 0x04048034 0x04048034 0x00100 0x00100 R E 0x4
    INTERP 0x000134 0x04048134 0x04048134 0x00013 0x00013 R 0x1
    LOAD 0x000000 0x04048000 0x04048000 0x008a4 0x008a4 R E 0x1000
    LOAD 0x0008a4 0x040498a4 0x040498a4 0x0011c 0x00120 RW 0x1000
    DYNAMIC 0x0008b0 0x040498b0 0x040498b0 0x000c8 0x000c8 RW 0x4
    NOTE 0x000148 0x04048148 0x04048148 0x00020 0x00020 R 0x4
    */

    fprintf(stdout, "Type Offset VirtAddr PhysAddr FileSiz MemSiz Flg Align\n");
    char *type = get_type(phdr->p_type);
    char *flags = get_flags(phdr->p_flags);
    char *align = get_align(phdr->p_align);
    fprintf(stdout, "%s 0x%06x 0x%08x 0x%08x 0x%05x 0x%05x %s %s\n", type, phdr->p_offset, phdr->p_vaddr, phdr->p_paddr, phdr->p_filesz, phdr->p_memsz, flags, align);

}

//task 1b
void print_all_info_extedended(Elf32_Phdr *phdr,int arg){
    //the same as print_all_info but with an additional column for the protection flags and maping flags
    
    char* protect_flag = calloc(1,100);
    char* map_flag = "";
    char* flag = get_flags(phdr->p_flags);

    if(phdr ->p_flags & PF_R)
        strcat(protect_flag, "PROT_READ ");
    if(phdr ->p_flags & PF_W)
        strcat(protect_flag, "PROT_WRITE ");
    if(phdr ->p_flags & PF_X)
        strcat(protect_flag, "PROT_EXEC ");
    if(phdr ->p_flags & PF_MASKOS)
        strcat(protect_flag, "PF_MASKOS ");
    if(phdr ->p_flags & PF_MASKPROC)
        strcat(protect_flag, "PF_MASKPROC ");
    else 
        strcat(protect_flag, "0");
    if(phdr->p_type == PT_LOAD)
    {
        if(phdr->p_flags & PF_R)
            map_flag = "MAP_PRIVATE";
        else
            map_flag = "MAP_SHARED";
    }
    else
        map_flag = "MAP_PRIVATE";
    fprintf(stdout, "Type Offset VirtAddr PhysAddr FileSiz MemSiz Flg Align Protect_flags Map_flags\n");
    char *type = get_type(phdr->p_type);
    fprintf(stdout, "%s 0x%06x 0x%08x 0x%08x 0x%05x 0x%05x %s %s %s %s\n", type, phdr->p_offset, phdr->p_vaddr, phdr->p_paddr, phdr->p_filesz, phdr->p_memsz, flag, get_align(phdr->p_align), protect_flag, map_flag);


   

}



char *get_flags(int flags){
    char *flags_str = malloc(4);
    if (flags & PF_R)
    {
        strcat(flags_str, "R");
    }
    if (flags & PF_W)
    {
        strcat(flags_str, "W");
    }
    if (flags & PF_X)
    {
        strcat(flags_str, "E");
    }
    return flags_str;
}

char *get_align(int align){
    switch (align)
    {
    case 0:
        return "0";
    case 1:
        return "1";
    case 2:
        return "2";
    case 4:
        return "4";
    case 8:
        return "8";
    case 16:
        return "16";
    case 32:
        return "32";
    case 64:
        return "64";
    case 128:
        return "128";
    case 256:
        return "256";
    case 512:
        return "512";
    case 1024:
        return "1024";
    case 2048:
        return "2048";
    case 4096:
        return "4096";
    case 8192:
        return "8192";
    default:
        return "0";
    }
}

//function that recives phdr->p_type and returns the corresponding string
char *get_type(int type){
    switch (type)
    {
    case PT_NULL:
        return "NULL";
    case PT_LOAD:
        return "LOAD";
    case PT_DYNAMIC:
        return "DYNAMIC";
    case PT_INTERP:
        return "INTERP";
    case PT_NOTE:
        return "NOTE";
    case PT_SHLIB:
        return "SHLIB";
    case PT_PHDR:
        return "PHDR";
    case PT_TLS:
        return "TLS";
    case PT_NUM:
        return "NUM";
    case PT_LOOS:
        return "LOOS";
    case PT_GNU_EH_FRAME:
        return "GNU_EH_FRAME";
    case PT_GNU_STACK:
        return "GNU_STACK";
    case PT_GNU_RELRO:
        return "GNU_RELRO";
    case PT_LOSUNW:
        return "LOSUNW";
    case PT_SUNWSTACK:
        return "SUNWSTACK";
    case PT_HISUNW:
        return "HISUNW";
    
    default:
        return "UNKNOWN";
    }
}

void load_phdr(Elf32_Phdr *phdr, int fd){
    if(phdr->p_type == PT_LOAD)
    {
        //load the segment into memory
        void *vir_address = (void *)(phdr->p_vaddr&0xfffff000);
        int status = (int)mmap(vir_address, phdr->p_memsz + (phdr->p_vaddr&0xfff),phdr->p_flags, MAP_PRIVATE|MAP_FIXED, fd, phdr->p_offset&0xfffff000);
        if(status == -1)
        {
            fprintf(stderr, "Error in load_phdr: \n"); 
            exit(1);
        }
        print_all_info(phdr, fd);
        
    }
  

}




int main(int argc, char **argv){
    //gets a single command line argument from stdin. The argument will be the file name of a 32bit ELF formatted executable.
    char file_name[100];
    // fprintf(stdout, "Enter the file name of a 32bit ELF formatted executable: ");
    // fgets(file_name, 100, stdin);
    // file_name[strlen(file_name)-1] = '\0';
    //file name is passed as a command line argument
    strcpy(file_name, argv[1]);
    

    //load the file into memory
    int fd = load_file(file_name);
    if (fd == -1)
    {
        return 0;
    }
    elf_header = (Elf32_Ehdr *) file_map;
    //check if the file is an ELF file from the first 4 bytes or not 32bit
    if (elf_header->e_ident[0] != 0x7f || elf_header->e_ident[1] != 'E' || elf_header->e_ident[2] != 'L' || elf_header->e_ident[3] != 'F' || elf_header->e_ident[4] != ELFCLASS32)
    {
        fprintf(stderr, "Error: Not a 32bit ELF file\n");
        munmap(file_map, file_stat.st_size);
        close(fd);
        return 0;
    }
    //iterate over program headers, for each one call foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *,int), int arg);
    // print : "Program header number i at address x" for each program header
    foreach_phdr(file_map, load_phdr, fd);
    Elf32_Ehdr *ehdr = (Elf32_Ehdr*) file_map;
    startup(argc-1,argv+1,(void*)ehdr->e_entry);

    munmap(file_map, file_stat.st_size);
    close(fd);
    return 0;
}