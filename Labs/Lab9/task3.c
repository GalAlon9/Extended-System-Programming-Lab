#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <elf.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>



typedef struct {
  char *name;
  void (*fun)();
} fun_desc;



int debug_mode = 0;
int curr_fd = -1;
struct stat file_stat;
void *file_map; //pointer to the start of the mapped file
Elf32_Ehdr *elf_header;
int hex_flag = 0;
int dec_flag = 0;
int oct_flag = 0;
Elf32_Ehdr *map_start; /* will point to the start of the memory mapped file */
Elf32_Shdr *section_header_table; /* will point to the section header table */
Elf32_Shdr *section_header_string_table; /* will point to the section header string table */
Elf32_Shdr *symb_table; /* will point to the symbol table */







void toggleDebugMode()
{
    if (debug_mode == 0)
    {
        debug_mode = 1;
        printf("Debug flag now on\n");
    }
    else
    {
        debug_mode = 0;
        printf("Debug flag now off\n");
    }
}

void Examine_ELF_File()
{
    printf("Please enter file name: ");
    char file_name[128];
    fscanf(stdin, "%s", file_name);
    curr_fd = load_file(file_name);
    if (curr_fd == -1)
    {
        printf("Error loading file \n");
        exit(1);
    }
    //check if file is ELF
    if (strncmp(file_map, "\x7f\x45\x4c\x46", 4) != 0)
    {
        printf("Not an ELF file \n");
        munmap(file_map, file_stat.st_size);
        close(curr_fd);
        curr_fd = -1;
        exit(1);
    }
    elf_header = (Elf32_Ehdr*)file_map;
    //1.print bytes 1-3 of the magic number
    printf("Magic: ");
    for (int i = 0; i < 3; i++)
    {
        printf("%02x ", ((unsigned char*)file_map)[i]);
    }
    printf("\n");
    //2.print data encoding scheme of the file
    printf("Data: ");
    if (((unsigned char*)file_map)[5] == 1)
    {
        printf("2's complement, little endian \n");
    }
    else if (((unsigned char*)file_map)[5] == 2)
    {
        printf("2's complement, big endian \n");
    }
    else
    {
        printf("Unknown data encoding \n");
    }
    //3.print the file's entry point
    printf("Entry point address: ");
    printf("%x \n", elf_header->e_entry);
    //4.print the program header table's file offset, in bytes
    printf("Start of program headers: ");
    printf("%d (bytes into file) \n", elf_header->e_phoff);
    //5.print the number of section header entries
    printf("Number of section headers: ");
    printf("%d \n", elf_header->e_shnum);
    //6.print the size of each section header entry, in bytes
    printf("Size of section headers: ");
    printf("%d (bytes) \n", elf_header->e_shentsize);
    //7.print the file offset in which the section header table resides, in bytes
    printf("Section header string table index: ");
    printf("%d \n", elf_header->e_shstrndx);
    //8.print the number of program header entries
    printf("Number of program headers: ");
    printf("%d \n", elf_header->e_phnum);
    //9.print the size of each program header entry, in bytes
    printf("Size of program headers: ");
    printf("%d (bytes) \n", elf_header->e_phentsize);

}

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

char *get_section_name(int index)
{
    switch (index)
    {
    case SHT_NULL: return "NULL";
    case SHT_PROGBITS: return "PROGBITS";
    case SHT_SYMTAB: return "SYMTAB";
    case SHT_STRTAB: return "STRTAB";
    case SHT_RELA: return "RELA";
    case SHT_HASH: return "HASH";
    case SHT_DYNAMIC: return "DYNAMIC";
    case SHT_NOTE: return "NOTE";
    case SHT_NOBITS: return "NOBITS";
    case SHT_REL: return "REL";
    case SHT_SHLIB: return "SHLIB";
    case SHT_DYNSYM: return "DYNSYM";
    case SHT_INIT_ARRAY: return "INIT_ARRAY";
    case SHT_FINI_ARRAY: return "FINI_ARRAY";
    case SHT_PREINIT_ARRAY: return "PREINIT_ARRAY";
    case SHT_GROUP: return "GROUP";
    case SHT_SYMTAB_SHNDX: return "SYMTAB_SHNDX";
    case SHT_NUM: return "NUM";
    case SHT_LOOS: return "LOOS";
    case SHT_GNU_ATTRIBUTES: return "GNU_ATTRIBUTES";
    case SHT_GNU_HASH: return "GNU_HASH";
    case SHT_GNU_LIBLIST: return "GNU_LIBLIST";
    case SHT_CHECKSUM: return "CHECKSUM";
    case SHT_LOSUNW: return "LOSUNW";
    case SHT_SUNW_COMDAT: return "SUNW_COMDAT";
    case SHT_SUNW_syminfo: return "SUNW_syminfo";
    case SHT_GNU_verdef: return "GNU_verdef";
    case SHT_GNU_verneed: return "GNU_verneed";
    default: return "UNKNOWN";
    }


}

void print_sections(){
    /**
     * Print Section Names should visit all section headers in the section header table, 
     * and for each one print its index, name, address, offset, size in bytes, and type number. 
     * Note that this is done for the file currently mapped, so if current fd is invalid, just print an error message and return.
     * The format should be:
     * [index] section_name section_address section_offset section_size  section_type
    */
    if (curr_fd == -1)
    {
        printf("Error: No file loaded \n");
        return;
    }
    Elf32_Shdr *section_header = (Elf32_Shdr*)(file_map + elf_header->e_shoff);
    Elf32_Shdr *str_table = (Elf32_Shdr*)(file_map + elf_header->e_shoff + elf_header->e_shstrndx * elf_header->e_shentsize);
    char *section_names = (char*)(file_map + section_header[elf_header->e_shstrndx].sh_offset);
    if (debug_mode == 1)
    {
        printf("section table adress: %p \n", section_header);
        printf("string table adress: %p \n", str_table);
        printf("section names offset: %d \n", section_header[elf_header->e_shstrndx].sh_offset);
    }
    for(size_t i = 0; i < elf_header->e_shnum; i++)
    {
        printf("[%d] %s %x %x %x %s \n", i, section_names + section_header[i].sh_name, section_header[i].sh_addr, section_header[i].sh_offset, section_header[i].sh_size, get_section_name(section_header[i].sh_type));
    }
}

void print_symbols(){
    /**
     * The new Print Symbols option should visit all the symbols in the current ELF file (if none, print an error message and return). 
     * For each symbol, print its index number, its name and the name of the section in which it is defined. 
     * (similar to readelf -s). Format should be:
     * [index] value section_index section_name symbol_name
    */
    if (curr_fd == -1)
    {
        printf("Error: No file loaded \n");
        return;
    }
    Elf32_Shdr *symbol_table = file_map + elf_header->e_shoff+(elf_header->e_shstrndx * elf_header->e_shentsize);
    for (size_t i = 0; i < elf_header->e_shnum; i++)
    {
         Elf32_Shdr *temp = file_map + elf_header->e_shoff+(i * elf_header->e_shentsize);
         if (temp->sh_type == SHT_SYMTAB)
         {
              symbol_table = temp;
              break;
         }
    }
    Elf32_Shdr *str_table = file_map + elf_header->e_shoff+(symbol_table->sh_link * elf_header->e_shentsize);
    Elf32_Shdr *shstr_table = file_map + elf_header->e_shoff+(elf_header->e_shstrndx * elf_header->e_shentsize);
    int num_of_symbols = symbol_table->sh_size / symbol_table->sh_entsize;
    printf("[index] |  value section_index |  section_name symbol_name \n");
    for (size_t i = 0; i < num_of_symbols; i++)
    {
        Elf32_Sym *symbol = file_map + symbol_table->sh_offset + (i * symbol_table->sh_entsize);
        char *symbol_name = file_map + str_table->sh_offset + symbol->st_name;
        char *section_name = file_map + shstr_table->sh_offset + symbol->st_shndx;
        printf("[%d] | %x %d | %s %s \n", i, symbol->st_value, symbol->st_shndx, section_name, symbol_name);
    }
}

void print_relocation(Elf32_Shdr *relocation, Elf32_Shdr *dynsym, Elf32_Shdr *dynstr,char *table_name){
    printf("Relocation section '%s' at offset 0x%x contains %d entries: \n", table_name, relocation->sh_offset, relocation->sh_size / relocation->sh_entsize);
    int num_of_relocations = relocation->sh_size / relocation->sh_entsize;
    printf("offset | info | type | symbol value | symbol name \n");
    for (size_t i = 0; i < num_of_relocations; i++)
    {
        Elf32_Rel *relocation_entry = file_map + relocation->sh_offset + (i * relocation->sh_entsize);
        Elf32_Sym *symbol = file_map + dynsym->sh_offset + (ELF32_R_SYM(relocation_entry->r_info) * dynsym->sh_entsize);
        char *symbol_name = file_map + dynstr->sh_offset + symbol->st_name;
        printf("%x | %x | %x | %x | %s \n", relocation_entry->r_offset, relocation_entry->r_info, ELF32_R_TYPE(relocation_entry->r_info), symbol->st_value, symbol_name);
    }
}


void relocate(){
    /*Print the content of all fields of all relocation tables entries, in hexadecimal format.
    This is similar to what readelf -r prints, except this feature prints the raw table data (i.e. without fetching symbol name strings).*/

    if (curr_fd == -1)
    {
        printf("Error: No file loaded \n");
        return;
    }
    Elf32_Shdr *str_table_start = file_map + elf_header->e_shoff+(elf_header->e_shstrndx * elf_header->e_shentsize); 
    Elf32_Shdr *relocation_dyn = str_table_start;
    Elf32_Shdr *relocation_plt = str_table_start;
    Elf32_Shdr *dynsym = str_table_start;
    Elf32_Shdr *dynstr = str_table_start;
    for (size_t i = 0; i < elf_header->e_shnum; i++)
    {
         Elf32_Shdr *temp = file_map + elf_header->e_shoff+(i * elf_header->e_shentsize);
         char *section_name = file_map + str_table_start->sh_offset + temp->sh_name;
         if (strcmp(section_name, ".rel.dyn") == 0)
         {
              relocation_dyn = temp;
         }
        if (strcmp(section_name, ".rel.plt") == 0)
        {
            relocation_plt = temp;
        }
        if (strcmp(section_name, ".dynsym") == 0)
        {
            dynsym = temp;
        }
        if (strcmp(section_name, ".dynstr") == 0)
        {
            dynstr = temp;
        }

    }
    print_relocation(relocation_dyn, dynsym, dynstr, ".rel.dyn");
    print_relocation(relocation_plt, dynsym, dynstr, ".rel.plt");
}


   



    

    
 


void quit()
{
    munmap(file_map, file_stat.st_size);
    close(curr_fd);
    exit(0);
}   



int main(int argc, char **argv)
{
    fun_desc menu[] = { { "Toggle Debug Mode", toggleDebugMode }, 
                        { "Examine ELF File", Examine_ELF_File },
                        { "Print Section Names", print_sections},
                        { "Print Symbols", print_symbols },
                        { "Relocate a given address", relocate},
                        { "Quit", quit }, { NULL, NULL } };
    
    size_t menu_size = sizeof(menu)/sizeof(menu[0]);
    
    while (1)
    {
        int input;
        printf("Please choose a function: \n");
        for (int i = 0; i < menu_size-1; i++)
        {
            printf("%d) %s \n", i, menu[i].name);
        }
        
        printf("Option: ");
        scanf("%d", &input);
        getchar();
        if (input < 0 || input > menu_size - 1)
        {
            printf("Not within bounds \n");
            quit();
        }
        printf("Within bounds \n");
        menu[input].fun();
        printf("\n\n");
    }
    free(file_map);

    return 0;
}