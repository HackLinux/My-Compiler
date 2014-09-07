#include "parser.h"
#include "lexer.h"
#include <string.h>
#include <stdlib.h>

/** An array of the different string values of keywords. */
char *keywords[] = {"and", "or", "+", "-", "*", "/", "lt", "eq", 
"function", "struct", "arrow", "assign", "if", 
"while", "for", "sequence", "intprint", "stringprint",
"readint"};
/** Sister array of keywords. Keeps track of the corresponding enum. */
int enums[] = {node_AND, node_OR, node_PLUS, node_MINUS, node_MUL, node_DIV,
node_LT, node_EQ, node_FUNCTION, node_STRUCT, 
node_ARROW, node_ASSIGN, node_IF, node_WHILE, node_FOR, 
node_SEQ, node_I_PRINT, node_S_PRINT, node_READ_INT};
/** A hashmap used for more efficient lookups of (keyword, enum) pairs. */
smap *keyword_str_to_enum;


int varC = 0;
int structC = 0;


void buildAstHelperNew(lexer *lex, AST* currAST);
void buildAstHelperAppend(lexer *lex, AST* currAST);
AST* createNewAST(lexer *lex);
AST_lst* createNewASTList(AST* new);
void freeList(AST_lst* list);
void checkHelper(AST_lst* list);
void checkNode(AST * currAST);
void gather_decls_list(AST_lst* list, char *env, int is_top_level);
void styleError(char* msg);
void invalidMalloc();

/** Initializes keyword_str_to_enum so that it contains a map
*  from the string value of a keyword to its corresponding enum. */
void initialize_keyword_to_enum_mapping();


void parse_init() {
    decls = smap_new();
    stack_sizes = smap_new();
    num_args = smap_new();
    strings = smap_new();
    keyword_str_to_enum = smap_new();
}

void parse_close() {
    smap_del_contents(decls);
    smap_del(decls);
    smap_del(stack_sizes);
    smap_del(num_args);
    smap_del(strings);
    smap_del(keyword_str_to_enum);
}


int numOpen = 0;

AST *build_ast (lexer *lex) {
/* TODO: Implement me. */
/* Hint: switch statements are pretty cool, and they work 
 *       brilliantly with enums. */

//if meet a '(' then create a new AST, if meet 'space' then fill in its child list.

//read_token(lex);
    AST *rootNode;

    if(peek_type(lex)==token_CLOSE_PAREN){
    fprintf(stderr, "ERROR: first char from file is CLOSE_PAREN\n");  // need to check????? need to unread??or just abandon?
    exit(1);
    }

// if(peek_type(lex)!=token_END){
if (peek_type(lex)==token_OPEN_PAREN){   
numOpen++;
read_token(lex);
// if(peek_type(lex)==token_CLOSE_PAREN||peek_type(lex)==token_OPEN_PAREN){
//         fprintf(stderr, "ERROR: cannot be () or ((\n");  // need to check????? need to unread??or just abandon?
//         exit(1);
//     }
rootNode = createNewAST(lex);
read_token(lex);
    if(peek_type(lex)==token_CLOSE_PAREN){
        fprintf(stderr, "ERROR: cannot be (function)\n");  // need to check????? need to unread??or just abandon?
        exit(1);
    }
    while(numOpen!=0){
//while(peek_type(lex)!=token_END){
        if(peek_type(lex)==token_OPEN_PAREN){
            numOpen++;
            buildAstHelperNew(lex, rootNode);
        }else if(peek_type(lex)!=token_CLOSE_PAREN){
            buildAstHelperAppend(lex, rootNode);
        }else if(peek_type(lex)==token_CLOSE_PAREN){
            numOpen--;
        }
    read_token(lex);
    }
    if(!strcmp(rootNode->val,"function") && rootNode->children->val->type==node_VAR){   // when root is "function"
    rootNode->children->val->type = node_CALL;
    }

if(numOpen!=0){
    fprintf(stderr, "ERROR: numOpenPren not equal to numClosePren\n");  
    exit(1);
}

}else if(peek_type(lex)==token_STRING||peek_type(lex)==token_INT){
    rootNode = createNewAST(lex);
}

return rootNode;
}


void buildAstHelperNew(lexer *lex, AST* currAST){

read_token(lex);
// if(peek_type(lex)==token_CLOSE_PAREN||peek_type(lex)==token_OPEN_PAREN){
//     fprintf(stderr, "ERROR: cannot be () or ((\n");
//         exit(1);
//     }
    AST *newAST = createNewAST(lex);
    AST_lst *newList = createNewASTList(newAST);
    // if(!(peek_type(lex)==token_KEYWORD||peek_type(lex)==token_NAME)){
    //     fprintf(stderr, "ERROR: first word is not a keyword\n");
    //     exit(1);
    // }
    if(currAST->last_child==NULL){
        currAST->children = newList;
        currAST->last_child= newList;
    }else{
    currAST->last_child->next = newList;
    currAST->last_child = newList;
    }
//printf("my value is %s, my children is %s, my last_child is %s\n", currAST->val, currAST->children->val->val, currAST->last_child->val->val);

    while(peek_type(lex)!=token_CLOSE_PAREN){
        read_token(lex);
        if(peek_type(lex)==token_OPEN_PAREN){
            numOpen++;


            buildAstHelperNew(lex, newAST);
        }else if(peek_type(lex)!=token_CLOSE_PAREN){
            buildAstHelperAppend(lex, newAST);
        }else if(peek_type(lex)==token_CLOSE_PAREN){
            numOpen--;

        }
    }
}



void buildAstHelperAppend(lexer *lex, AST* currAST){
    AST *newAST = createNewAST(lex);
    AST_lst *newList = createNewASTList(newAST);
    if(peek_type(lex)==token_KEYWORD){
        fprintf(stderr, "ERROR: this word should not be a keyword\n");
        exit(1);
    }
    if(currAST->last_child==NULL){
        currAST->children = newList;
        currAST->last_child = newList;
    }else{
        currAST->last_child->next = newList;
        currAST->last_child = newList;
    }

//printf("my value is %s, my children is %s, my last_child is %s\n", currAST->val, currAST->children->val->val, currAST->last_child->val->val);
}




AST* createNewAST(lexer* lex){
    AST* newNode = (AST*)malloc(sizeof(AST));
    if (!newNode) {
    invalidMalloc();
    }

    token_type t = peek_type(lex);
    char* value = (char*)calloc((strlen(lex->buffer)+1),1);
    strcpy(value, peek_value(lex));

    switch (t){
        case token_INT: 
        newNode->type = node_INT;
        break;
        case token_STRING: 
        newNode->type = node_STRING;
        break;
        case token_KEYWORD: 
        newNode->type = lookup_keyword_enum(value);
        break;
        case token_NAME: 
        newNode->type = node_VAR;

        break;
//   default:
    }

    newNode->val = value;
    newNode->children = NULL;
    newNode->last_child = NULL;

    return newNode;
    }


AST_lst* createNewASTList(AST* new){
AST_lst* newList = (AST_lst*)malloc(sizeof(AST_lst));
if (!newList) {
    invalidMalloc();
    }
newList->val = new;
newList->next = NULL;
return newList;
}


// free the last child and move the last child pointer  AST: val, children, last_child, parent; AST_Lst: val, next
void free_ast (AST *ptr) { 
/* TODO: Implement me. */
if(ptr->children){
    freeList(ptr->children);
}
free(ptr->val);
//free(ptr->children);
free(ptr);
}


void freeList(AST_lst* list){
if(list->val){
    free_ast(list->val);
}
if(list->next){
    freeList(list->next);
}
//free(list->val);
free(list);
}




void checkHelper(AST_lst* list){
if(list->val){
    check_tree_shape(list->val);
}
if(list->next){
    checkHelper(list->next);
}
checkNode(list->val);
}



void check_tree_shape(AST *ptr) {
/* TODO: Implement me. */
/* Hint: This function is just asking to be table-driven */



if(ptr!=NULL){
checkNode(ptr);

if(ptr->children){
    checkHelper(ptr->children);
}


}
}




void checkNode(AST * currAST){

size_t num = AST_lst_len(currAST->children);
node_type currType = currAST->type;

char* msg = "ERROR: bad style from check_tree_shape function";

switch(currType){
    case node_AND:
    case node_OR:
    case node_PLUS:
    case node_MINUS:
    case node_MUL:
    case node_LT:
    case node_EQ:
    case node_DIV:
    case node_WHILE:
    if(num!=2){
        styleError(msg);
    }
    break;

    case node_IF:
    if(num!=3){
        styleError(msg);
    }
    break;
    
    case node_ASSIGN:
    if(num!=2||!(currAST->children->val->type==node_VAR||currAST->children->val->type==node_ARROW)){
        styleError(msg);
    }
    break;
    case node_FUNCTION:
    case node_STRUCT:
    case node_SEQ:
    if(num==0){
        styleError(msg);
    }
    break;
    
    case node_I_PRINT:
    if(num!=1){
        styleError(msg);
    }
    break;
    case node_S_PRINT:
    if(num!=1){
        styleError(msg);
    }
    break;

    case node_READ_INT:
    if(num!=0){
        styleError(msg);
    }
    break;

    case node_FOR:
    if(num!=4){
        styleError(msg);
    }
    break;

    case node_ARROW:
    if(num!=2){
        styleError(msg);
    }

    currAST = currAST->children->val;
    if(!(currAST->type==node_STRUCT || currAST->type==node_VAR || currAST->type==node_ARROW)){
        styleError(msg);
    }
        size_t numChild = AST_lst_len(currAST->children);      //(arrow (struct 3 (+ 2 3) "hi") 2) would return "hi"
        if(currAST->type!=node_STRUCT||numChild>(num-1)){
            styleError(msg);
        }
        break;
//     default:
    }

}


void styleError(char* msg) {
    fprintf(stderr, "%s\n", msg);
    exit(1);
}


  void invalidMalloc(){
    fprintf(stderr, "failed malloc!");
    exit(1);
  }
    

void gather_decls(AST *ast, char *env, int is_top_level) {
    /* TODO: Implement me. */
    /* Hint: switch statements are pretty cool, and they work 
     *       brilliantly with enums. */
    int varC = 0;
    int structC = 0;

if(ast!=NULL){

AST* currAST = ast;

//char name[80];


if(currAST->type == node_CALL){
    char* funcName = (char*)malloc(strlen(currAST->val)+2);
    if (!funcName) {
    invalidMalloc();
    }
    funcName = strcpy(funcName, currAST->val);
    sprintf(funcName, "%s$", funcName);
    int numArgs = (int)AST_lst_len(currAST->children);
    smap_put(num_args, funcName, numArgs);      // do i need to cast???????????????

    AST_lst* inputList = currAST->children;
    int varcounter = 1;
    while(inputList){
        char* inputName = (char*)malloc(strlen(inputList->val->val)+strlen(funcName) +2);
        sprintf(inputName, "%s$%s", funcName, inputList->val->val);
        smap_put(stack_sizes, inputName, varcounter);
        inputList = inputList->next;
        varcounter++;
    }

}else if(currAST->type == node_ASSIGN){     
 //There can be three different kinds of variable inside a function.
//If the variable name is equal to the parameter, you want to update that.
//else, if the variable was already defined above (not in a function), you want to update that one.
//Finally, if the variable wasn't defined above, and if it's not equal to the parameter, you need to store it locally.
 char *varName = (char*)malloc(strlen(currAST->val)+strlen(env)+2);
    if (!varName) {
    invalidMalloc();
    }
    sprintf(varName, "%s$%s", env, currAST->children->val->val);
   // char* value = currAST->children->next->val->val;
    smap_put(decls, varName, varC);
    varC++;
}else if(currAST->type == node_STRUCT){                           // struct!!!!!!!!!!
    int numChildren = (size_t)AST_lst_len(currAST->children);
    char *structName = (char*)malloc(strlen(currAST->val)+strlen(env)+5);
    if (!structName) {
    invalidMalloc();
    }
    sprintf(structName, "%s$%s$%d", env, "sct", numChildren);  // cast??????????????
    smap_put(decls, structName, structC);   // not sure if the counter is correct!!!!!!!!!!!do struct share counter with var??????
    structC++;

    // char *structAddr = (char*)malloc(strlen(currAST->val)+1);
    // if (!structAddr) {
    // invalidMalloc();
    // }
    // sprintf(structAddr, "%s$%s$%d", env, "sct", 0); //store addr of the struct
    // smap_put(decls, structName, structC);
    // structC++;
}


if(currAST->children){
    gather_decls_list(currAST->children, env, is_top_level);
}
}
}


void gather_decls_list(AST_lst* list, char *env, int is_top_level){
if(list->val){
    gather_decls(list->val, env, is_top_level);
}

if(list->next){
    gather_decls_list(list->next, env, is_top_level);
}
}




//hash all the assign, struct in the function and in the global.
    // val to be $value be global, %value to be in function. key to be the name of the var.
    // hash num of args in function into num_arg map: value to be num, key to be function name.
    //


node_type lookup_keyword_enum(char *str) {
    if (smap_get(keyword_str_to_enum, keywords[0]) == -1) {
       initialize_keyword_to_enum_mapping();
   }
   return smap_get(keyword_str_to_enum, str);
}

void initialize_keyword_to_enum_mapping() {
/* Note that enums is an *array*, not a pointer, so this
 * sizeof business is reasonable. */
size_t num_keywords = sizeof(enums) / sizeof(int);
for (size_t i = 0; i < num_keywords; i += 1) {
   smap_put(keyword_str_to_enum, keywords[i], enums[i]);
}
}

size_t AST_lst_len(AST_lst *lst) {
int num_fields = 0;
while (lst) {
   num_fields += 1;
   lst = lst->next;
}
return num_fields;
}


smap *decls; //struct, variable assign
smap *stack_sizes; // to store the position of input arguments.
smap *num_args; //f
smap *strings; 
