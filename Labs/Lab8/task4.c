#include <stdio.h>

int digit_cnt(char *str)
{
    int count = 0;
    while (*str != '\0')
    {
        if (*str >= '0' && *str <= '9')
        {
            count++;
        }
        str++;
    }
    return count;
}

int main(int argc, char *argv[])
{
    return 0;
}