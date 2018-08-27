#include <stdio.h>
#include <stdlib.h>

/* On Windows editline functionality is redundant, we replace it with this */
#ifdef _WIN32
#include <string.h>
#define BUFFER_SIZE 2048

static char buffer[BUFFER_SIZE];

/* Dummy functions which mimic editline */
char* readline(char* prompt) {
    fputs(prompt, stdout);
    fgets(buffer, BUFFER_SIZE, stdin);
    /* We later use free() on this output */
    char* copy = malloc(strlen(buffer)+1);
    strcpy(copy, buffer);
    copy[strlen(copy)-1] = '\0';
    return copy;
}

void add_history(char* arg) {}

/* Otherwise, proceed as normal */
#else
#include <editline/readline.h>
#include <editline/history.h>
#endif

int main(int argc, char** argv) {

    /* Print Version and other info */
    puts("Interactive Sep Version 0.01");
    puts("Press Ctrl+c to Exit\n");

    /* Loops until program is killed */
    while (1) {

        /* Print prompt and get input */
        char* input = readline(">>> ");

        /* Add input to history, to be retrieved with arrow keys */
        add_history(input);

        /* Echo input */
        puts(input);

        free(input);

    }

    return 0;
}
