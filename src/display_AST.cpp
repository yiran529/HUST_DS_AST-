#include<stdio.h>
#include "display_AST.h"
#include "get_token.h"
#include "syntax_analysis.h"

void display_indent(int indent) {
    char* one_indent = "    ";
    for(int i = 1; i <= indent; i++) printf("%s", one_indent);
}

void display_var_def(AST_NODE* cur_node, int indent);

void display_statement_seq(AST_NODE* cur_node, int indent) {
    if(cur_node == NULL) return;
    display_indent(indent);
    switch(cur_node-> first_child-> type){
        case LOCAL_VAR_DEF:  printf("Local var definition: \n"); 
                       display_var_def(cur_node-> first_child, indent + 1);
                       break;
    }
    display_statement_seq(cur_node-> first_child-> next_sibling, indent);
}

void display_type(TOKEN_KIND kind) {
    switch(kind){
        case CHAR:   printf("char");  break;
        case INT:    printf("int");   break;
        case SHORT:  printf("short"); break;
        case LONG:   printf("long");  break;
        case FLOAT:  printf("float"); break;
        case DOUBLE: printf("double");break;
    }
}

void display_formal_param_seq(AST_NODE* cur_node, int indent) {
    if(cur_node == NULL) return;
    
    display_indent(indent);
    printf("type: ");
    display_type((TOKEN_KIND)cur_node-> first_child-> first_child-> word.kind);
    printf("  ");
    printf("name: ");
    printf("%s", cur_node-> first_child-> first_child-> next_sibling-> word.data);
    printf("\n");
    display_formal_param_seq(cur_node-> first_child-> next_sibling, indent);
}

void display_func_def(AST_NODE* cur_node, int indent) {
    display_indent(indent);
    printf("type: ");
    display_type((TOKEN_KIND)cur_node-> first_child-> word.kind);
    printf("\n");

    display_indent(indent);
    printf("function name: ");
    printf("%s\n", cur_node-> first_child-> next_sibling-> word.data);

    display_indent(indent);
    printf("Formal parametre: \n");
    if(cur_node->first_child-> next_sibling-> next_sibling-> type == FORMAL_PARAM_SEQ)
        display_formal_param_seq(cur_node-> first_child-> next_sibling-> next_sibling, indent + 1);
    else {
        display_indent(indent + 1);
        printf("None\n");
    }

    AST_NODE* &fourth_child = cur_node-> first_child-> next_sibling-> next_sibling-> next_sibling;
    if(fourth_child == NULL) return;
    display_indent(indent);
    printf("Compound statement: \n");
    display_statement_seq(fourth_child-> first_child, indent + 1);
}

void display_var_list(AST_NODE* cur_node, int indent) {
    if(cur_node == NULL) return;
    display_indent(indent);
    printf("ID:  ");
    printf("%s\n", cur_node-> first_child-> word.data);
    display_var_list(cur_node-> first_child-> next_sibling, indent);
}

void display_var_def(AST_NODE* cur_node, int indent) {
    display_indent(indent);
    printf("type: ");
    display_type((TOKEN_KIND)cur_node-> first_child-> word.kind);
    printf("\n");

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
                          display_var_def(cur_node, indent + 1);
                          break;
        case FUNC_DEF:    display_indent(indent);
                          printf("Function definition: \n");
                          display_func_def(cur_node, indent + 1);
    }
}