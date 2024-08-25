#include<stdio.h>
#include "display_AST.h"
#include "get_token.h"
#include "syntax_analysis.h"

void display_indent(int indent) {
    char* one_indent = "    ";
    for(int i = 1; i <= indent; i++) printf("%s", one_indent);
}

void display_var_list(AST_NODE* cur_node, int indent) {
    if(cur_node == NULL) return;
    display_indent(indent);
    printf("ID:  ");
    printf("%s\n", cur_node-> first_child-> word.data);
    display_var_list(cur_node-> first_child-> next_sibling, indent);
}

void display_ext_var_def(AST_NODE* cur_node, int indent) {
    display_indent(indent);
    printf("type: ");
    switch(cur_node-> first_child-> word.kind){
        case CHAR:   printf("char\n");  break;
        case INT:    printf("int\n");   break;
        case SHORT:  printf("short\n"); break;
        case LONG:   printf("long\n");  break;
        case FLOAT:  printf("float\n"); break;
        case DOUBLE: printf("double\n");break;
    }
    display_indent(indent);
    printf("variables name:\n");
    display_var_list(cur_node-> first_child-> next_sibling, indent + 1);
}

void display_AST(AST_NODE* cur_node, int indent) {
    if(cur_node == NULL) return;
    
    switch(cur_node-> type){
        case PROGRAM:     printf("Program starts:\n"); 
                          display_AST(cur_node-> first_child, indent + 1);
                          break;
        case EXT_DEF_SEQ: //if(cur_node-> first_child) printf("Extern definition:\n"); 
                          display_AST(cur_node-> first_child, indent);
                          display_AST(cur_node-> first_child-> next_sibling, indent);
                          break;
        case EXT_VAR_DEF: display_indent(indent);
                          printf("Extern variable definition:\n"); 
                          display_ext_var_def(cur_node, indent + 1);
                          break;
        //case FUNC_DEF: display_func_def(cur_node);
    }
}