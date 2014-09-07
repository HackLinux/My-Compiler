#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "lexer.h"
#include "util/util.h"
#include "parser.h"

#define INIT_BUFFER_SIZE 256
#define DOUBLE_BUFFER_SIZE 512

/** An array of the different string values of keywords. */
char *keyW[] = {"and", "or", "+", "-", "*", "/", "lt", "eq", 
"function", "struct", "arrow", "assign", "if", 
"while", "for", "sequence", "intprint", "stringprint",
"readint"};






void init_lex(lexer *luthor) {
    luthor->file = NULL;
    luthor->buffer = NULL;
    luthor->type = token_SENTINEL;
    luthor->buff_len = 0;
}

void open_file(lexer *lex, char *filename) {
    if (lex) {
     lex->file = fopen(filename, "r");
     if (!lex->file) {
         fatal_error("Could not read input file.\n");
     }
     lex->buff_len = INIT_BUFFER_SIZE;
     lex->buffer = safe_calloc(INIT_BUFFER_SIZE * sizeof(char));
 }
}

void close_file(lexer *lex) {
    if (lex) {
     fclose(lex->file);
     free(lex->buffer);
     lex->buff_len = 0;
     lex->buffer = NULL;
 }
}

void read_token(lexer *lex) {
    /* TODO: Implement me. */
    /* HINT: fgetc() and ungetc() could be pretty useful here. */
    if(lex==NULL){
        fprintf(stderr, "ERROR: read_token passed a NULL lexer.\n");
        return;
    }


    if(lex->file){
        int c;
        do {
            c = fgetc(lex->file);
    } while ((c == ' ') || (c == '\n')||(c == '\t')||(c == 13));    //stop at a meaningful char!


    
    
    if (c == '(') {
        (lex->buffer)[0] = c;
        (lex->buffer)[1] = '\0';
        lex->type = token_OPEN_PAREN;
        lex->buff_len = 1;
       // printf("token_OPEN_PAREN\n");
        return;
    }
    else if (c == ')') {
        (lex->buffer)[0] = c;
        (lex->buffer)[1] = '\0';
        lex->type = token_CLOSE_PAREN;
        lex->buff_len = 1;
       // printf("token_CLOSE_PAREN\n");
        return;
    }

    else if (c == EOF) { 
        (lex->buffer)[0] = c;
        (lex->buffer)[1] = '\0'; 
        lex->type = token_END;
        lex->buff_len = 1;
      // printf("token_END\n");
        return;
    }

    // else if (c == 'N'){
    //     c = fgetc(lex->file);
    //     if( c == 'o'){
    //         lex->buffer = "0";
    //         lex->type = token_NONE;
    //         lex->buff_len = 1;
    //         printf("token_NONE(%s)\n", lex->buffer);

    //     }else{
    //         ungetc(c, lex->file);
    //     }
    // }

// Integer:
    else if((c>='0')&&(c<='9')){
        int i = 0;
        while(c!=' '&&c!='('&&c!=')') {
            if(lex->buff_len > INIT_BUFFER_SIZE){    //expand buffer!!!!
                lex->buffer = realloc(lex->buffer, sizeof(char)*DOUBLE_BUFFER_SIZE);
                if (!lex->buffer) {
                    invalidMalloc();
                }
            }

            (lex->buffer)[i] = c;
            c = fgetc(lex->file); 
            i++;
            lex->buff_len = i;
        }
        if(c=='('||c==')'){
            ungetc(c, lex->file);
        }
        (lex->buffer)[i] = '\0';
        lex->buff_len = i;
        lex->type = token_INT;
       // printf("token_INT(%s)\n", lex->buffer);
        return;
    } 

// minus sign or negative int:
    else if(c=='-'){
        (lex->buffer)[0] = c;
        c = fgetc(lex->file);

        if((c>='0')&&(c<='9')){
            int i = 1;                           //same code with INT!!
            while(c!=' '&&c!='('&&c!=')') {
            if(lex->buff_len > INIT_BUFFER_SIZE){    //expand buffer!!!!
                lex->buffer = realloc(lex->buffer, sizeof(char)*DOUBLE_BUFFER_SIZE);
                if (!lex->buffer) {
                    invalidMalloc();
                }
            }

            (lex->buffer)[i] = c;
            c = fgetc(lex->file); 
            i++;
            lex->buff_len = i;
        }
        if(c=='('||c==')'){
            ungetc(c, lex->file);
        }
        lex->buffer[i] = '\0';
        lex->buff_len = i;
        lex->type = token_INT;
       // printf("token_INT(%s)\n", lex->buffer);
        return;
    }
     else{                                // minus sign!!
        lex->buffer[1] = '\0';
        lex->buff_len = 1;
        lex->type = token_KEYWORD;
       // printf("token_KEYWORD(%s)\n", lex->buffer);
        return;
    }
}

//String:
else if(c=='"'){
    (lex->buffer)[0] = c;
    int i = 1;
    while((c = fgetc(lex->file))!='"'){
            if (c == EOF || c == ')' || c == ' ') {   // error case!!
                ungetc(c, lex->file);
                (lex->buffer)[i] = '\0';
                fprintf(stderr, "ERROR: the string does not have an ending\n");
                exit(1);
            }

            if(lex->buff_len > INIT_BUFFER_SIZE){    //expand buffer!!!!
                lex->buffer = realloc(lex->buffer, sizeof(char)*DOUBLE_BUFFER_SIZE);
                if (!lex->buffer) {
                    invalidMalloc();
                }
            }

            (lex->buffer)[i] = c;
            i++;
            lex->buff_len = i;
        }
        (lex->buffer)[i] = c;
        (lex->buff_len) = i+1;
        lex->buffer[i+1] = '\0';
        lex->type = token_STRING;
       // printf("token_STRING(%s)\n", lex->buffer);
        return;
    }
// Keywords:

    int j = 0;
    while(c!=' '&&c!='('&&c!=')') {
            if(lex->buff_len > INIT_BUFFER_SIZE){    //expand buffer!!!!
                lex->buffer = realloc(lex->buffer, sizeof(char)*DOUBLE_BUFFER_SIZE);
                if (!lex->buffer) {
                    invalidMalloc();
                }
            }

            (lex->buffer)[j] = c;
            c = fgetc(lex->file); 
            j++;
            lex->buff_len = j;
        }
        if(c=='('||c==')'){
            ungetc(c, lex->file);
        }
        lex->buffer[j] = '\0';
        lex->buff_len = j;

        int k = 0;
        while (k<19){
            if(strcmp(lex->buffer, keyW[k])){
                k++;
            }else{
                lex->type = token_KEYWORD;
              //  printf("token_KEYWORD(%s)\n", lex->buffer);
                return;
            }
        }

// if it is not keyword, should be name:

        // char* checker = lex->buffer;
        // while(checker){
        //     if(!((c>='0'&&c<='9')||(c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c=='_'))){
        //         styleError("invalid char!");
        //     }
        //     checker++;
        // }

        if(!strcmp(lex->buffer, "None")){
            lex->buffer[0] = '0';
            lex->buffer[1] = '\0';
            lex->type = token_INT;
            lex->buff_len = 1;
        }else{

            lex->type = token_NAME;
       // printf("token_NAME(%s)\n", lex->buffer);
            return;
        }
    }

}


token_type peek_type(lexer *lex) {
    if (!lex) {
     return token_SENTINEL;
 }
 if (lex->type == token_SENTINEL) {
     read_token(lex);
 }
 return lex->type;
}

char *peek_value(lexer *lex) {
    if (!lex) {
     return NULL;
 }
 if (lex->type == token_SENTINEL) {
     read_token(lex);
 }
 return lex->buffer;
}
