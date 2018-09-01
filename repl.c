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


/*Create enumeration of possible error types */
enum { SERR_DIV_ZERO, SERR_BAD_OP, SERR_BAD_NUM };

/* Create enumeration of possible sval types */
enum { SVAL_NUM, SVAL_ERR };

/* Declare sval struct */
typedef struct {
    int type;
    long num;
    int err;
} sval;

/* Number type sval constructor */
sval sval_num(long x) {
    sval new;
    new.type = SVAL_NUM;
    new.num = x;
    return new;
}

/* Error type sval constructor */
sval sval_err(int x) {
    sval new;
    new.type = SVAL_ERR;
    new.err = x;
    return new;
}

/* Print an sval value */
void sval_print(sval value) {
    switch (value.type) {
        /* If value is a number, print it */
        case SVAL_NUM:
            printf("%li", value.num);
            break;

        /* If value is an error, output appropiate message */
        case SVAL_ERR:
            switch (value.err) {
                case SERR_DIV_ZERO:
                    printf("Error: Division by zero");
                    break;
                case SERR_BAD_OP:
                    printf("Error: Invalid operator");
                    break;
                case SERR_BAD_NUM:
                    printf("Error: Invalid number");
                    break;
                default:
                    printf("Error: Unknown error");
            } break;

        /* Impossible case */
        default:
            printf("Critical error: Invalid sval type");
    }
}

/* Convenience function, adds a newline to sval_print */
void sval_println(sval value) { sval_print(value); putchar('\n'); }


/* Use operator string to see which operation to perform */
sval eval_op(sval x, char* op, sval y) {
    /* If either value is an error, return it */
    if (x.type == SVAL_ERR) { return x; }
    if (y.type == SVAL_ERR) { return y; }

    /* Otherwise, evaluate */
    if (strcmp(op, "+") == 0) { return sval_num(x.num + y.num); }
    if (strcmp(op, "-") == 0) { return sval_num(x.num - y.num); }
    if (strcmp(op, "*") == 0) { return sval_num(x.num * y.num); }
    if (strcmp(op, "/") == 0) {
        /* If second operand is zero return error */
        return y.num == 0 ? sval_err(SERR_DIV_ZERO) : sval_num(x.num / y.num);
    }

    return sval_err(SERR_BAD_OP);
}

/* Evaluate an expression */
sval eval(mpc_ast_t* tree) {

    /* If node is tagged as number, return it */
    if (strstr(tree->tag, "number")) {
        /* Check if there is some error in conversion */
        errno = 0;
        long x = strtol(tree->contents, NULL, 10);
        return errno != ERANGE ? sval_num(x) : sval_err(SERR_BAD_NUM);
    }

    /* Otherwise retrieve the operator */
    char* op = tree->children[1]->contents;

    /* We store the first argument in 'x' */
    sval x = eval(tree->children[2]);

    /* Iterate the remaning children, applying operator. */
    int i = 3;
    while (strstr(tree->children[i]->tag, "expr")) {
        x = eval_op(x, op, eval(tree->children[i]));
        i++;
    }

    return x;
}

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
            /* On succesfull parse, return the value of the computation */
            sval output = eval(result.output);
            sval_println(output);
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
