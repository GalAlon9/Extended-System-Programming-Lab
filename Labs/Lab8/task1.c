#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>



typedef struct {
  char debug_mode;
  char file_name[128];
  int unit_size;
  unsigned char mem_buf[10000];
  size_t mem_count;
  char display_mode;
} state;

typedef struct {
  char *name;
  void (*fun)(state*);
} fun_desc;



int debug = 0;


void toggleDebugMode(state *s)
{
    if (s->debug_mode == 0)
    {
        s->debug_mode = 1;
        printf("Debug flag now on\n");
    }
    else
    {
        s->debug_mode = 0;
        printf("Debug flag now off\n");
    }
}

void setFileName(state *s)
{
    printf("Please enter file name: ");
    fgets(s->file_name, 128, stdin);
    s->file_name[strlen(s->file_name) - 1] = '\0';
    if (s->debug_mode == 1)
    {
        printf("Debug: file name set to %s \n", s->file_name);
    }

}

void setUnitSize(state *s)
{
    int unit_size;
    printf("Please enter unit size: ");
    scanf("%d", &unit_size);
    if (unit_size != 1 && unit_size != 2 && unit_size != 4)
    {
        printf("Invalid unit size \n");
    }
    else
    {
        s->unit_size = unit_size;
        if (s->debug_mode == 1)
        {
            printf("Debug: set size to %d \n", s->unit_size);
        }
    }
}

void quit(state *s)
{
    printf("quitting");
    exit(0);
}

void loadIntoMemory(state *s)
{   
    int location=0;
    int length = 0;
    //check if file name is empty
    if (s->file_name[0] == '\0')
    {
        printf("File name is empty \n");
        return;
    }
    //open file
    FILE *fp = fopen(s->file_name, "r+");
    if (fp == NULL)
    {
        printf("Error opening file \n");
        return;
    }
    //ask the user for address and length
    printf("Please enter <location> <length> \n");
    scanf("%x %d", &location, &length);

    if (s->debug_mode == 1)
    {
        printf("Debug: file name is %s \n", s->file_name);
        printf("Debug: location is %d \n", location);
        printf("Debug: length is %d \n", length);
    }
    //copy the data from the file to the memory buffer
    fseek(fp, location, SEEK_SET);
    fread(s->mem_buf, s->unit_size, length, fp);
    s->mem_count = length*s->unit_size;
    fprintf(stdout, "Loaded %d units into memory \n", length);
    fclose(fp);
}

void toggleDisplayMode(state *s)
{
    if (s->display_mode == 0)
    {
        s->display_mode = 1;
        printf("Display flag now off, hexadecimal representation \n");
    }
    else
    {
        s->display_mode = 0;
        printf("Display flag now on, decimal representation \n");
        
    }

}
char *format_decimal(int unit_size)
{ 
    //unit size can be 2 or 4
    // if unit size is 2, return "%#hd\n"
    // if unit size is 4, return "%#d\n"
    if (unit_size == 2) return "%#hd \t";
    else return "%#d \t";

}

char *format_hexadecimal(int unit_size)
{
    //unit size can be 2 or 4
    //if unit size is 2, return %#hx\n
    //if unit size is 4, return %#x\n
    if (unit_size == 2) return "%#hx \t";
    else return "%#x \t";
}

void memoryDisplay(state *s)
{
    /*Write the function for the "Memory Display" option:
This option displays u units of size unit_size starting at address addr in memory. Unit_size is already defined in state, but u and addr should be queried from the user by this function. u will be given in decimal and addr in hexadecimal. Entering a value of 0 for addr is a special case, in which the memory to be displayed starts at your mem_buf.*/
    fprintf(stdout, "Enter <num of units> <adress>: \n");
    int num_of_units = 0;
    int address = 0;
    scanf("%d %x", &num_of_units, &address);
    if (s->debug_mode == 1)
    {
        printf("Debug: number of units is %d \n", num_of_units);
        printf("Debug: address is %d \n", address);
    }
    //Entering a value of 0 for addr is a special case, in which the memory to be displayed starts at your mem_buf.
    if (address == 0) address  = (int)s->mem_buf;
    if (s->display_mode == 0) 
    {
        printf("Decimal\n=========\n");
        for (int i = 0; i < num_of_units; i++)
        {
            fprintf(stdout,format_decimal(s->unit_size), *((int*)address));
            address += s->unit_size;
            printf("\n");
        }
    
    }
    else
    {
        printf("Hexadecimal\n=========\n");
        for (int i = 0; i < num_of_units; i++)
        {
            fprintf(stdout,format_hexadecimal(s->unit_size), *((int*)address));
            address += s->unit_size;
            printf("\n");
        }
        
    }   
    printf("\n");  
}

void saveIntoFile(state *s)
{
    
    int source_address = 0;
    int target_location = 0;
    int length = 0;
    printf("Please enter <source-address> <target-location> <length> \n");
    scanf("%x %x %d", &source_address, &target_location, &length);
    if (s->debug_mode == 1)
    {
        printf("Debug: source address is %d \n", source_address);
        printf("Debug: target location is %d \n", target_location);
        printf("Debug: length is %d \n", length);
    }
    //open file
    FILE *fp = fopen(s->file_name, "r+");
    if (fp == NULL)
    {
        printf("Error opening file \n");
        return;
    }
    //copy the data from the file to the memory buffer
    fseek(fp,0,SEEK_SET);
    fseek(fp, target_location, SEEK_SET);
    if (source_address == 0) fwrite(&(s->mem_buf), s->unit_size, length, fp);
    else fwrite(&(source_address), s->unit_size, length, fp);
    fclose(fp);

}

void memoryModify(state *s)
{
    int location = 0;
    int value = 0;
    printf("Please enter <location> <value> \n");
    scanf("%x %x", &location, &value);
    if (s->debug_mode == 1)
    {
        printf("Debug: location is %d \n", location);
        printf("Debug: value is %d \n", value);
    }
    //check that the location chosen to be modified, given the currentunit size, is valid, and act accordingly
    if (location > s->mem_count)
    {
        printf("Error: location is out of bounds \n");
        return;
    }
    

    //replace a unit of size unit_size at location with value
    memcpy(&(s->mem_buf[location]), &value, s->unit_size);
}



int main (int argc, char **argv)
{
    state* s = calloc (1 , sizeof(state));

    fun_desc menu[] = { { "Toggle Debug Mode", toggleDebugMode }, { "Set File Name", setFileName }, { "Set Unit Size", setUnitSize },{ "Load Into Memory", loadIntoMemory },{ "Toggle Display Mode", toggleDisplayMode },{ "Memory Display", memoryDisplay },{ "Save Into File", saveIntoFile },{ "Memory Modify", memoryModify},
                                { "Quit", quit } ,{ NULL, NULL } };

    size_t menu_size = sizeof(menu)/sizeof(menu[0]);
    
    while (1)
    {
        int input;
        printf("Please choose a function: \n");
        for (int i = 0; i < menu_size-1; i++)
        {
            printf("%d) %s \n", i, menu[i].name);
        }
        if (s->debug_mode == 1)
        {
            printf("Debug: unit size is %d \n", s->unit_size);
            printf("Debug: file name is %s \n", s->file_name);
            printf("Debug: memory count is %d \n", s->mem_count);
        }
        printf("Option: ");
        scanf("%d", &input);
        getchar();
        if (input < 0 || input > menu_size - 1)
        {
            printf("Not within bounds \n");
            quit(s);
        }
        printf("Within bounds \n");
        menu[input].fun(s);
        printf("\n\n");
    }
      
}
    