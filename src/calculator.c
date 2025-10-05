#include "calculator.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

void calculate(EXPRESSION * exp)  {

    switch (exp->operator) {
    case '+':
        exp->result = exp->op1 + exp->op2;
        strcpy(exp->message,"Sum is: ");
        break;

    case '-':
        exp->result = exp->op1 - exp->op2;
        strcpy(exp->message,"Subtraction is: ");
        break;

    case '*':
        exp->result = exp->op1 * exp->op2;
        strcpy(exp->message,"Multiplication is: ");
        break;

    case '/':
        if (exp->op2 == 0) { // division by zero
            exp->result = 0;
            strcpy(exp->message,"KABOOM!");
        }
        else {
            exp->result = exp->op1 / exp->op2;
            strcpy(exp->message,"Division is: ");
        }
        break;

        
    default:
        exp->result = 0;
        strcpy(exp->message,"Calculator Error");
        break;
    
    }
}

int invalid_range(EXPRESSION exp) {
    if (exp.op1 < 0 || exp.op1 > 10000 || exp.op2 < 0 || exp.op2 > 10000)
        return 1;
    else
        return 0;
}

int quit(char *input_str) {    
    if (strcmp(input_str, "quit") == 0) // input is "quit"
        return 1;
    else
        return 0;
};

int format_input (char * input_str, EXPRESSION * exp) { 
    // check if input has structure "op1 operator op2, otherwise will write "invalid exp" message" 
    float op1, op2;
    char operator;

    printf("%s\n", input_str);
    if (sscanf(input_str, "%f%c%f", &op1, &operator, &op2) != 3){ // this will do the trick for now
        strcpy(exp->message,"Error in the format of the expression!\n");
        return -1; // Parsing failed due to too much elements
    }
    if (strchr("+-*/",operator) == NULL) {
            strcpy(exp->message,"Invalid operator!\n");
            return -1; // Operator invalid
    }
    exp->op1 = op1;
    exp->op2 = op2;
    exp->operator = operator;
    return 0;
}

void clean_input(char *input_str) {
    // remove white spaces  and lowers all cases
    size_t j = 0;
    for (size_t i = 0; input_str[i]; i++) {
        if (input_str[i] != ' ' && input_str[i] != '\n'){ 
            if ((input_str[i] >= '0' && input_str[i] <= '9') || input_str[i] == '.'
                 || input_str[i] == '+' || input_str[i] == '-' 
                 || input_str[i] == '*' || input_str[i] == '/'){
                    
                input_str[j++] = tolower(input_str[i]);
            }        
        }        
    }
    
    input_str[j] = '\0';
}
