#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"

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

    /* Create some parsers */
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Sep = mpc_new("sep");

    /* Define them */
    mpca_lang(MPCA_LANG_DEFAULT,
        "                                                       \
          number    : /-?[0-9]+/ ;                              \
          operator  : '+' | '-' | '*' | '/' ;                   \
          expr      : <number> | '(' <operator> <expr>+ ')' ;   \
          sep       : /^/ <operator> <expr>+ /$/ ;              \
        ",
        Number, Operator, Expr, Sep);

    /* Print Version and other info */
    puts("Interactive Sep Version 0.01");
    puts("Press Ctrl+c to Exit\n");

    /* Loops until ctrl-c */
    while (1) {

        /* Print prompt and get input */
        char* input = readline(">>> ");

        /* Add input to history, to be retrieved with arrow keys */
        add_history(input);

        /* Attempt to parse input */
        mpc_result_t result;
        if (mpc_parse("<stdin>", input, Sep, &result)) {
            /* On success print the AST */
            mpc_ast_print(result.output);
            mpc_ast_delete(result.output);
        } else {
            /* Otherwise Print the Error */
            mpc_err_print(result.error);
            mpc_err_delete(result.error);
        }

        free(input);

    }

    /* Undefine and delete parsers */
    mpc_cleanup(4, Number, Operator, Expr, Sep);

    return 0;
}
