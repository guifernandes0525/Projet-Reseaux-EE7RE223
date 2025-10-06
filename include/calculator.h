#ifndef CALCULATOR_H
#define CALCULATOR_H

#define MSG_LEN 64

typedef struct expression {
    float op1, op2, result;
    char operator;
    char message[MSG_LEN];
    
} EXPRESSION;


void calculate(EXPRESSION * expression);

int invalid_range(EXPRESSION exp);

int format_input (char * input_str, EXPRESSION  * exp, char* IP_client);

void clean_input(char *input_str);

int quit(char *input_str);

#endif
