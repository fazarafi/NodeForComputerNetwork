#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

static void *set_malloc(size_t size) {
    void* ptr;
    //assert(size < 100);

    ptr = malloc(size);
    if (ptr == NULL) {
        abort();
    }
    return ptr;
}

int GetNumber(const char *str) {
    while (!(*str >= '0' && *str <= '9') && (*str != '-') && (*str != '+')) str++;
    int number;
    if (sscanf(str, "%d", &number) == 1) {
        return number;
    }
        // No int found
        return -1; 
}

int* readfile() {
    FILE * fp;
    char * line = NULL;
    int j;
    int* mem;

    mem = set_malloc(3);
    int i = 0;
    size_t len = 0;
    ssize_t read;

    fp = fopen("/proc/meminfo", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
        //printf("Retrieved line of length %zu :\n", read);
        printf("%s", line);
        if (i<3) {
            mem[i] = GetNumber(line);
            printf("total %d\n",mem[i]);
            i++;

        }
        //printf("memory = %d\n", GetNumber(line));
        
        
    }
 
    fclose(fp);
    if (line)
        free(line);
 

    return mem;
}

int main(void)
{
    int* Mem;
    int i;
    Mem = set_malloc(3);
    for (i=0;i<3;i++) {
        printf("ea %d\n",Mem[i]);
    }   
    Mem = readfile();
    
    for (i=0;i<3;i++) {
        printf("ea %d\n",Mem[i]);
    }
    for (i=0;i<3;i++) {
        printf("ea %d\n",Mem[i]);
    }
    

}