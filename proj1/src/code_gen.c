#include "code_gen.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/** A counter to prevent the issuance of duplicate labels. */
unsigned label_count = 0;
/** True iff the data segment has already been partially printed. */
int data_seg_opened = 0;
/** True iff the text segment has already been partially printed. */
int text_seg_opened = 0;

 int scounter = 0;
 int StrCount = 0;
 int times = 0;
 char func[80];


void strings_ast (AST *ptr);
void strings_list (AST_lst* list);
void findKeyError();
void concatString(char* varN, char* str);
void emit_func(AST *ast);


void concatString(char* varN, char* str){
    sprintf(varN, "$%s", str);
}

void findKeyError() {
    fprintf(stderr, "the var is not defined\n");
    exit(1);
}



void strings_ast (AST *ptr) { 
if(ptr->type == node_STRING){
    printf("s$%d: .asciiz %s\n", scounter, ptr->val);   // string start from 0
    scounter++;
}
if(ptr->children){
    strings_list(ptr->children);
}
}


void strings_list (AST_lst* list){
if(list->val){
    strings_ast(list->val);
}
if(list->next){
    strings_list(list->next);
}
}


    //look through the tree and hash all the string in the map. val to string, key to string's labal $a b c d ...
    //.data
//s1: .asciiz "Hello"
//s2: .asciiz "Bye"

void emit_strings(AST *ast) {
    /* TODO: Implement me. */
 if(ast!=NULL){

// AST* currAST = ast;

// char str[80];
// sprintf(str, "s$%d", counter);

printf("    .data\n");
strings_ast(ast);
}
}




void emit_static_memory(AST *ast) {
    /* TODO: Implement me. */

    // see the hashmap and emit all the global
// int varCounter = 0;
// sprintf(staticVar, "$%s", currAST->children->val->val);
if (ast->type==node_ASSIGN){
    if(ast->children->next->val->type == node_INT){
        printf("$%s:    .word    %s\n", ast->children->val->val, ast->children->next->val->val);
    }else if(ast->children->next->val->type == node_STRING){
        printf("$%s:    .data    %s\n", ast->children->val->val, ast->children->next->val->val);
    }else if(ast->children->next->val->type == node_STRUCT){
       
        int numfield = (size_t)AST_lst_len(ast->children->next->val->children);
        int sctSpace = numfield*4;
        printf("$%s:    .space    %d\n", ast->children->val->val, sctSpace);
        
    }else{
        printf("$%s:    .space    %d\n", ast->children->val->val, 40);
    }    
}

if(ast->type==node_SEQ){
    while(ast->children){
    emit_static_memory(ast->children->val);
    ast = ast->children->val;
    }
}
    
}



void emit_main(AST *ast) {

    /* TODO: Implement me. */
// AST* currAST = ast;
// node_type currType = ast->type;
    times++;

//int sctCMain = 0;

    if(times==1){
        printf("    .text\n");
        printf("main:\n");
    }
    switch(ast->type){

        case node_INT:
            printf("    addiu $t0, $0, %s\n", ast->val);
        break;

        case node_STRING:
        {
            char * s = "s";
            strcat(s, "$");
            printf("    la $t0 %s%d\n", s, StrCount);  // string counter
            StrCount++;
            break;
        }

    case node_I_PRINT:
        emit_main(ast->children->val);
        printf("    add $a0, $t0, $0\n");
        printf("    li $v0, 1\n");
        printf("    syscall\n");
        printf("    move $t0, $0\n"); //return NONE
    break;

    case node_READ_INT:
        printf("    li $v0, 5\n");
        printf("    syscall\n");
        printf("    move $t0, $0\n"); //return NONE
    break;

    case node_S_PRINT:
    // printf("    la $a0 s$%d\n", StrCount);  // string counter
        emit_main(ast->children->val);
        printf("    add $a0, $t0, $0\n");
        printf("    li $v0, 4\n");
        printf("    syscall\n");
    break;

    case node_AND:  // evaluate $t2 and $t1 are all true, $t0 is one. otherwise 0.
        emit_main(ast->children->val); //1st
        printf("    move $t1, $t0\n");

        emit_main(ast->children->next->val); // 2rd
        printf("    move $t2, $t0\n");

        printf("    beq $t1, $0, evaluateAnd\n");
        printf("    addiu $t1, $0, 1\n");

        printf("    beq $t2, $0, evaluateAnd\n");
        printf("    addiu $t2, $0, 1\n");

        printf("evaluateAnd:\n");
        printf("    and $t0, $t1, $t2\n");
        // printf("    move $t1, $0\n");
        // printf("    move $t2, $0\n");
    break;


    case node_OR:   // evaluate $t2 or $t1 is true, $t0 is one. otherwise 0.
        emit_main(ast->children->val); //1st
        printf("    move $t1, $t0\n");

        emit_main(ast->children->next->val); // 2rd
        printf("    move $t2, $t0\n");

        printf("    or $t0, $t1, $t2\n");
        // printf("    move $t1, $0\n");
        // printf("    move $t2, $0\n");
    break;

    case node_PLUS:
    
        printf("    addiu $sp, $sp, -4\n");
        emit_main(ast->children->val); //1st
        printf("    sw $t0, 0($sp)\n");

        emit_main(ast->children->next->val); // 2rd
        printf("    move $t2, $t0\n");
        printf("    lw $t3, 0($sp)\n");
        printf("    add $t0, $t3, $t2\n");
        printf("    addiu $sp $sp 4\n");
    break;

    case node_MINUS:
    printf("    addiu $sp, $sp, -4\n");
    emit_main(ast->children->val); //1st
    printf("    sw $t0, 0($sp)\n");

    emit_main(ast->children->next->val); // 2rd
    printf("    move $t2, $t0\n");
    printf("    lw $t3, 0($sp)\n");
    printf("    subu $t0, $t3, $t2\n");
    printf("    addiu $sp $sp 4\n");
    break;

    case node_MUL:
    printf("    addiu $sp, $sp, -4\n");
    emit_main(ast->children->val); //1st
    printf("    sw $t0, 0($sp)\n");

    emit_main(ast->children->next->val); // 2rd
    printf("    move $t2, $t0\n");
    printf("    lw $t3, 0($sp)\n");
    printf("    mult $t0, $t3, $t2\n");
    printf("    addiu $sp $sp 4\n");
    break;

    case node_DIV:
    printf("    addiu $sp, $sp, -4\n");
    emit_main(ast->children->val); //1st
    printf("    sw $t0, 0($sp)\n");

    emit_main(ast->children->next->val); // 2rd
    printf("    move $t2, $t0\n");
    printf("    lw $t3, 0($sp)\n");
    printf("    div $t0, $t3, $t2\n");
    printf("    addiu $sp $sp 4\n");
    break;

    case node_LT:
    emit_main(ast->children->val); //1st
    printf("    addiu $sp, $sp, -4\n");
    printf("    sw $t0, 0($sp)\n");

    emit_main(ast->children->next->val); // 2rd
    printf("    move $t1, $t0\n");
    printf("    lw $t2 0($sp)\n");
    printf("    slt $t0, $t2, $t1\n");
    printf("    addiu $sp $sp 4\n");
    // printf("    move $t1, $0\n");
    // printf("    move $t2, $0\n");
    break;

    case node_EQ:
    emit_main(ast->children->val); //1st
    printf("    addiu $sp, $sp, -4\n");
    printf("    sw $t0, 0($sp)\n");

    emit_main(ast->children->next->val); // 2rd
    printf("    move $t1, $t0\n");
    printf("    lw $t2 0($sp)\n");
    printf("    seq $t0, $t2, $t1\n");
    printf("    addiu $sp $sp 4\n");
    // printf("    move $t1, $0\n");
    // printf("    move $t2, $0\n");
    break;
    

    case node_IF:
    emit_main(ast->children->val); //predicate
    printf("    beq $t0, $0, false\n");
    emit_main(ast->children->next->val); //if true
    
    printf("false:\n");
    emit_main(ast->children->next->next->val); // if false  
    break;  


    case node_WHILE:
    break;
    

    case node_FOR:
    break;
    
    
    case node_VAR:
    {
        char varN[80];
        concatString(varN, ast->val);
        if(smap_get(decls, varN)!=-1){
            printf("    la $t1, %s\n", varN);
            printf("    lw $t0, 0($t1)\n");
  //  printf("    move $t1, $0\n");
        }else{
            findKeyError();
        }
        break;
    }

    case node_ASSIGN:
    {
      if(ast->children->val->type == node_VAR){
        if(ast->children->next->val->type == node_VAR){
            char varN[80];
            concatString(varN, ast->children->next->val->val);

            char varN2[80];
            concatString(varN, ast->children->val->val);

            printf("    la $t1, %s\n", varN);
            printf("    lw $t0, 0($t1)\n");
            printf("    la $t2, %s\n", varN2);
            printf("    sw $t0, 0($t2)\n");
        // printf("    move $t1, $0\n");
        // printf("    move $t2, $0\n");
        }else if(ast->children->next->val->type == node_STRUCT){
             char varN[80];
            concatString(varN, ast->children->val->val);

            printf("    la $t6, %s\n", varN);
            AST_lst* currChild = ast->children->next->val->children;
            int num = 0;
            while(currChild){
                emit_main(currChild->val);
                printf("    sw $t0, %d($t6)\n", num*4);
                printf("    move $t0, $0\n");
                currChild=currChild->next;
                num++;
            }
      //  printf("    move $t1, $0\n");
    // }
    // else if(ast->children->next->val->type == node_INT){
    //     emit_main(ast->children->next->val);
        }else if(ast->children->next->val->children){    // (assign x (+ 2 3))
            char varN[80];
            concatString(varN, ast->children->val->val);

            printf("    la $t5, %s\n", varN);
            emit_main(ast->children->next->val);
            printf("    sw $t0, 0($t5)\n");
            printf("    move $t5, $0\n");

        }
    }
    // else if(ast->children->val->type == node_ARROW)){
    // emit_main(ast->children->val);
    break;
    }




    // case node_FUNCTION:
    // emit_functions(ast);
    // break;


    case node_CALL:  //call!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    {
        AST_lst* currInputList = ast->children;
        int inpu = 0;
    while(currInputList){  // use s register instead of a for input args!!!!!!!!!!!!!!!!!!!
        printf("    addiu $s%d, $0, %s\n", inpu, currInputList->val->val);
        currInputList = currInputList->next;
        inpu++;
        printf("###########\n");
    }
printf("!!!!!!!!!!!!!!\n");
    
    sprintf(func, "%s$", ast->children->val->val);
    printf("    jal %s\n", func);

    break;
}



case node_STRUCT:
break;

case node_ARROW:
{
    if(ast->children->val->type == node_VAR){
        int extract = 4*(atoi(ast->children->next->val->val)-1);
        char varN[80];
            concatString(varN, ast->children->val->val);

        printf("    la $t1, %s\n", varN);
        printf("    lw $t0, %d($t1)\n", extract);
        printf("    move $t1, $0\n");
    }else if(ast->children->val->type == node_STRUCT){
        int target = atoi(ast->children->next->val->val);
        AST_lst* currList = ast->children->val->children;
        while(target>0){
            currList = currList->next;
            target--;
        }
        emit_main(currList->val);
    }
    break;
}


case node_SEQ:
{
    AST_lst* SEQnext = ast->children;
    while(SEQnext){
        emit_main(SEQnext->val);
        SEQnext = SEQnext->next;
    }
    break;
}


}
// if(ast->val=="0"){
//     printf("    move $t0, $0\n");
// }
}

void emit_exit() {
    printf("    li $v0 10\n");
    printf("    syscall\n");
}
void emit_functions(AST *ast) {  // ast is the node_CALL
    /* TODO: Implement me. */
    // see thro the hashmap, know how many space i should use: local var, inputs, sp.
    // store all the input arguements and sp. jr ra at the every end.
    


if(ast->type == node_FUNCTION){
    
    sprintf(func, "%s$", ast->children->val->val);

    emit_func(ast->children->val);
    emit_func(ast->children->next->val);

    AST_lst* InputList = ast->children->val->children;
    int inputNum = 0;
    int backplace = (inputNum+1)*4;
    while(InputList){
        printf("    lw $s%d, %d($sp)\n", inputNum, backplace);
        InputList = InputList->next;
        inputNum++;
    }
    printf("    addiu $sp $sp 40\n");
    printf("    jr $ra\n");
}
}

void emit_func(AST *ast) {


 switch(ast->type){
    case node_CALL:
    {
    printf("%s:\n", func);
    printf("    addiu $sp $sp -40\n");

    AST_lst* currInputList = ast->children;
    int inp = 0;
    int place = (inp+1)*4;
    while(currInputList){
        printf("    sw $s%d, %d($sp)\n", inp, place);
        currInputList = currInputList->next;
        inp++;
    }
    }
    break;


    case node_INT:
    printf("    addiu $t0, $0, %s\n", ast->val);
    break;

    case node_STRING:
    {
    // char * s = "s";
    // strcat(s, "$");
    printf("    la $t0 s$%d\n", StrCount);  // string counter
    StrCount++;
    }
    break;

    case node_I_PRINT:
    emit_main(ast->children->val);
    printf("    add $a0, $t0, $0\n");
    printf("    li $v0, 1\n");
    printf("    syscall\n");
    printf("    move $t0, $0\n"); //return NONE
    break;

    case node_READ_INT:
    printf("    li $v0, 5\n");
    printf("    syscall\n");
    printf("    move $t0, $0\n"); //return NONE
    break;

    case node_S_PRINT:
     printf("    la $a0 s$%d\n", StrCount);  // string counter
    //emit_main(ast->children->val);
    printf("    add $a0, $t0, $0\n");
    printf("    li $v0, 4\n");
    printf("    syscall\n");
    break;

    case node_AND:  // evaluate $t2 and $t1 are all true, $t0 is one. otherwise 0.
    emit_main(ast->children->val); //1st
    printf("    move $t1, $t0\n");

    emit_main(ast->children->next->val); // 2rd
    printf("    move $t2, $t0\n");

    printf("    beq $t1, $0, evaluateAnd\n");
    printf("    addiu $t1, $0, 1\n");

    printf("    beq $t2, $0, evaluateAnd\n");
    printf("    addiu $t2, $0, 1\n");

    printf("evaluateAnd:\n");
    printf("    and $t0, $t1, $t2\n");
    // printf("    move $t1, $0\n");
    // printf("    move $t2, $0\n");
    break;


    case node_OR:   // evaluate $t2 or $t1 is true, $t0 is one. otherwise 0.
    emit_main(ast->children->val); //1st
    printf("    move $t1, $t0\n");

    emit_main(ast->children->next->val); // 2rd
    printf("    move $t2, $t0\n");

    printf("    or $t0, $t1, $t2\n");
    // printf("    move $t1, $0\n");
    // printf("    move $t2, $0\n");
    break;

    case node_PLUS:
    printf("    addiu $sp, $sp, -4\n");
    emit_main(ast->children->val); //1st
    printf("    sw $t0, 0($sp)\n");

    emit_main(ast->children->next->val); // 2rd
    printf("    move $t2, $t0\n");
    printf("    lw $t3, 0($sp)\n");
    printf("    add $t0, $t3, $t2\n");
    printf("    addiu $sp $sp 4\n");
    break;

    case node_MINUS:
    printf("    addiu $sp, $sp, -4\n");
    emit_main(ast->children->val); //1st
    printf("    sw $t0, 0($sp)\n");

    emit_main(ast->children->next->val); // 2rd
    printf("    move $t2, $t0\n");
    printf("    lw $t3, 0($sp)\n");
    printf("    subu $t0, $t3, $t2\n");
    printf("    addiu $sp $sp 4\n");
    break;

    case node_MUL:
   printf("    addiu $sp, $sp, -4\n");
    emit_main(ast->children->val); //1st
    printf("    sw $t0, 0($sp)\n");

    emit_main(ast->children->next->val); // 2rd
    printf("    move $t2, $t0\n");
    printf("    lw $t3, 0($sp)\n");
    printf("    mult $t0, $t3, $t2\n");
    printf("    addiu $sp $sp 4\n");
    break;

    case node_DIV:
    printf("    addiu $sp, $sp, -4\n");
    emit_main(ast->children->val); //1st
    printf("    sw $t0, 0($sp)\n");

    emit_main(ast->children->next->val); // 2rd
    printf("    move $t2, $t0\n");
    printf("    lw $t3, 0($sp)\n");
    printf("    div $t0, $t3, $t2\n");
    printf("    addiu $sp $sp 4\n");

    case node_LT:
    emit_main(ast->children->val); //1st
    printf("    addiu $sp, $sp, -4\n");
    printf("    sw $t0, 0($sp)\n");

    emit_main(ast->children->next->val); // 2rd
    printf("    move $t1, $t0\n");
    printf("    lw $t2 0($sp)\n");
    printf("    slt $t0, $t2, $t1\n");
    printf("    addiu $sp $sp 4\n");
    // printf("    move $t1, $0\n");
    // printf("    move $t2, $0\n");
    break;

    case node_EQ:
    emit_main(ast->children->val); //1st
    printf("    addiu $sp, $sp, -4\n");
    printf("    sw $t0, 0($sp)\n");

    emit_main(ast->children->next->val); // 2rd
    printf("    move $t1, $t0\n");
    printf("    lw $t2 0($sp)\n");
    printf("    seq $t0, $t2, $t1\n");
    printf("    addiu $sp $sp 4\n");
    // printf("    move $t1, $0\n");
    // printf("    move $t2, $0\n");
    break;
    

    case node_IF:
    emit_func(ast->children->val); //predicate
    printf("    beq $t0, $0, false\n");
    emit_func(ast->children->next->val); //if true
    
    printf("false:\n");
    emit_func(ast->children->next->next->val); // if false    
    break;


    case node_WHILE:
    break;
    

    case node_FOR:
    break;

    

    case node_VAR:
    {
    char varName[80];
    sprintf(varName, "%s%s", func, ast->val);
    int varPlace = smap_get(stack_sizes, varName);
    if (varPlace!=-1){  // on stack
    printf("    lw $t0 %d($sp)\n", varPlace*4);
    }else{
        char varGN[80];
        concatString(varGN, ast->val);
        int varGlobal = smap_get(decls, varGN);
        if(varGlobal!=-1){  // global var
            printf("    la $t1, %s\n",varGN);
            printf("    lw $t0, 0($t1)\n");
          //  printf("    move $t1, $0\n");
        }else{
            //printf("!!my type%s\n", ast->type);
            findKeyError();
        }
    }
    break;
    }
    
    
    case node_ASSIGN:  // change the input values
    {
    if(ast->children->val->type == node_VAR){
        char varNameAss[80];
        sprintf(varNameAss, "%s%s", func, ast->val);
        int varPlaceAss = smap_get(stack_sizes, varNameAss);
        if (varPlaceAss!=-1){
        // printf("    lw $t0, %d($sp)\n", varPlaceAss*4);
        //     if(ast->children->next->val->type == node_INT){
        //         printf("    addiu $t0, $0, %s\n", ast->children->next->val->val);
        //     }else{
                emit_func(ast->children->next->val);
            //}
        printf("    sw $t0, %d($sp)\n", varPlaceAss*4);
        }
    }
    break;
    }
   
    
    case node_STRUCT:
    break;

    
    case node_ARROW:
    {
    if(ast->children->val->type == node_VAR){

    char sctName[80];
    concatString(sctName, ast->children->val->val);
    
        if (smap_get(decls, sctName)!=-1){
            int field = 4*(atoi(ast->children->next->val->val)-1);

            printf("    la $t1, %s\n", sctName);
            printf("    lw $t0, %d($t1)\n", field);
        }
    
    }else if(ast->children->val->type == node_STRUCT){
        int target = atoi(ast->children->next->val->val);
        AST_lst* currList = ast->children->val->children;
        while(target>0){
            currList = currList->next;
            target--;
        }
        emit_func(currList->val);
    }
    }
    break;

    case node_SEQ:
    {
    AST_lst* SEQnext = ast->children;
    while(SEQnext){
        emit_func(SEQnext->val);
        SEQnext = SEQnext->next;
    }
    }
    break;
    
}
// if(ast->val=="0"){
//     printf("    move $t0, $0\n");
// }



}

