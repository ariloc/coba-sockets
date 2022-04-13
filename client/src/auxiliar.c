#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "auxiliar.h"


int parse_response_code(char *code){
    return 100*(code[0]-'0') + 10*(code[1]-'0') + (code[2]-'0');
}

void read_string_buf(char *text){
    fgets(text, MAXSTRLEN, stdin);
    text[strcspn(text, "\r\n")] = '\0';
}