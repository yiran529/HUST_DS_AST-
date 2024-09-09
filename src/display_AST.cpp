#include<stdio.h>
#include "display_AST.h"
#include "get_token.h"
#include "syntax_analysis.h"
void display_statement_seq(AST_NODE* cur_node, int indent);
void display_indent(int indent) {
    char* one_indent = "  ";
    for(int i = 1; i <= indent; i++) printf("%s", one_indent);
}


void display_var_def(AST_NODE* cur_node, int indent);
void display_expression_tree(AST_NODE* cur_node, int indent);

void display_for_loop(AST_NODE* &cur_node, int indent) {
    display_indent(indent);
    printf("init: \n");
    if((*get_child(cur_node, 1))-> first_child) 
        display_expression_tree((*get_child(cur_node, 1))-> first_child, indent + 1);
    else {display_indent(indent + 1); printf("none\n");}

    display_indent(indent);
    printf("condition: \n");
    if((*get_child(cur_node, 2))-> first_child) 
        display_expression_tree((*get_child(cur_node, 2))-> first_child, indent + 1);
    else {display_indent(indent + 1); printf("none\n");}

    display_indent(indent);
    printf("increment: \n");
    if((*get_child(cur_node, 3))-> first_child) 
        display_expression_tree((*get_child(cur_node, 3))-> first_child, indent + 1);
    else {display_indent(indent + 1); printf("none\n");}

    display_indent(indent);
    printf("sub-statement: \n");
    if((*get_child(cur_node, 4))-> type == COMPOUND_STATEMENT)
        display_statement_seq((*get_child(cur_node, 4))-> first_child, indent + 1);
    else display_statement_seq((*get_child(cur_node, 4)), indent + 1);
}

void display_while_loop(AST_NODE* cur_node, int indent) {
    display_indent(indent);
    printf("condition: \n");
    display_indent(indent + 1);
    printf("expression tree: \n");
    display_expression_tree(cur_node-> first_child-> first_child, indent + 1);
    display_indent(indent);
    printf("sub-statement: \n");
    if((*get_child(cur_node, 2))-> type == COMPOUND_STATEMENT)
        display_statement_seq((*get_child(cur_node, 2))-> first_child, indent + 1);
    else display_statement_seq((*get_child(cur_node, 2)), indent + 1);
}

void display_actual_param(AST_NODE* cur_node, int indent) {
    if(cur_node == NULL) return;
    display_indent(indent);
    printf("expression tree: \n");
    display_expression_tree(cur_node-> first_child-> first_child, indent); // 
    display_actual_param(*get_child(cur_node, 2), indent); 
}

void display_function_call(AST_NODE* cur_node, int indent) {
    display_indent(indent);
    printf("function call: \n");
    display_indent(indent + 1);
    printf("function name: ");
    printf("%s\n", cur_node-> first_child-> word-> data);
    
    display_indent(indent + 1);
    printf("actual parameter: \n");
    if((*get_child(cur_node, 2))-> first_child) // 有实参
        display_actual_param(*get_child(cur_node, 2), indent + 2);
    else { // 没有实参时显示none
        display_indent(indent + 2);
        printf("none\n");
    }
}

void display_expression_tree(AST_NODE* cur_node, int indent) { //一定要注意第一层调用要从EXPRESSION结点的第一个子节点开始
    if(cur_node == NULL) return;
    
    if(cur_node-> type == WORD){
        display_indent(indent);
        printf("%s\n", cur_node-> word-> data);
        if(cur_node-> word-> array_info != NULL) {
            display_indent(indent);
            printf("[\n");
            display_indent(indent + 1);
            printf("expresssion tree: \n");
            display_expression_tree(cur_node-> word-> array_info-> first_child, indent + 1);
            display_indent(indent);
            printf("]\n");
        }
        display_expression_tree(cur_node-> first_child, indent + 1);
        if(cur_node-> first_child) // 不能保证第一个孩子一定存在
            display_expression_tree(cur_node-> first_child-> next_sibling, indent + 1);
    }
    else display_function_call(cur_node, indent);
    
}

void display_conditional_statement(AST_NODE* cur_node, int indent) {
    display_indent(indent);
    printf("condition: \n");
    display_expression_tree(cur_node-> first_child-> first_child, indent + 1);

    display_indent(indent);
    printf("sub-statement: \n");
    if(cur_node-> first_child-> next_sibling-> type == STATEMENT_SEQ)
        display_statement_seq(*get_child(cur_node, 2), indent + 1);
    else display_statement_seq((*get_child(cur_node, 2))-> first_child, indent + 1);

    if(*get_child(cur_node, 3)){
        display_indent(indent);
        printf("else: \n");
        if((*get_child(cur_node, 3))-> type == COMPOUND_STATEMENT)
            display_statement_seq((*get_child(cur_node, 3))-> first_child, indent);
        else display_statement_seq((*get_child(cur_node, 3)), indent);
    }
}

void display_statement(AST_NODE* cur_node, int indent) {
    display_indent(indent);
    switch(cur_node-> type){
        case LOCAL_VAR_DEF:  printf("Local var definition: \n"); 
                       display_var_def(cur_node, indent + 1);
                       break;
        case EXPRESSION: printf("expression tree: \n");
                         if(cur_node-> first_child) // 不是空语句
                         display_expression_tree(cur_node-> first_child, indent); 
                         else {display_indent(indent); printf("none \n"); }
                         break;
        case CONDITIONAL_STATEMENT: printf("Conditional statement: \n");
                        display_conditional_statement(cur_node, indent + 1);
                        break;
        case RETURN_STATEMENT: printf("Return statement: \n");
                         display_indent(indent + 1);
                         printf("Expression tree: \n");
                         display_expression_tree(cur_node-> first_child-> first_child, indent + 1);
                         break;
        case WHILE_LOOP: printf("While loop: \n");
                         display_while_loop(cur_node, indent + 1);
                         break; 
        case FOR_LOOP:  printf("For loop: \n");
                        display_for_loop(cur_node, indent + 1);
                        break;
        case BREAK_STATEMENT: printf("break statement\n"); break;
        case CONTINUE_STATEMENT: printf("continue statement\n"); break;
        case COMPOUND_STATEMENT: printf("compound statement: \n");
                                display_statement_seq(cur_node-> first_child, indent + 1);
                                break;
        case DO_WHILE_LOOP:
                        printf("do-while loop\n");
                        display_indent(indent + 1);
                        printf("sub-statement:\n");
                        display_statement(*get_child(cur_node, 1), indent + 2);
                        display_indent(indent + 1);
                        printf("condition: \n");
                        display_statement(*get_child(cur_node, 2), indent + 2);
                        break;
    }
}

void display_statement_seq(AST_NODE* cur_node, int indent) {
    if(cur_node == NULL) return;
    display_statement(cur_node-> first_child, indent);
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
        case STRING: printf("string");break;
        case VOID:   printf("void");  break;
    }
}

void display_formal_param_seq(AST_NODE* cur_node, int indent) {
    if(cur_node == NULL) return;
    
    display_indent(indent);
    printf("type: ");
    display_type((TOKEN_KIND)cur_node-> first_child-> first_child-> word-> kind);
    printf("  ");
    printf("name: ");
    printf("%s", cur_node-> first_child-> first_child-> next_sibling-> word-> data);
    printf("\n");
    display_formal_param_seq(cur_node-> first_child-> next_sibling, indent);
}

void display_func_def(AST_NODE* cur_node, int indent) {
    display_indent(indent);
    printf("type: ");
    if(cur_node-> first_child-> word-> type_prefix) // 如果有类型修饰符
        printf("%s ", cur_node-> first_child-> word-> type_prefix-> word-> data);
    display_type((TOKEN_KIND)cur_node-> first_child-> word-> kind);
    printf("\n");

    display_indent(indent);
    printf("function name: ");
    printf("%s\n", cur_node-> first_child-> next_sibling-> word-> data);

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
    printf("%s\n", cur_node-> first_child-> word-> data);
    display_var_list(cur_node-> first_child-> next_sibling, indent);
}

void display_var_def(AST_NODE* cur_node, int indent) {
    display_indent(indent);
    printf("type: ");
    if(cur_node-> first_child-> word-> type_prefix) // 如果有类型修饰符
        printf("%s ", cur_node-> first_child-> word-> type_prefix-> word-> data);
    display_type((TOKEN_KIND)cur_node-> first_child-> word-> kind);
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
                          break;
        case MACRO_DEFINE_STATEMENT:
                          display_indent(indent);
                          printf("Macro definition: \n");
                          display_indent(indent + 1);
                          printf("macro name: ");
                          printf("%s\n", cur_node-> first_child-> word-> data);
                          display_indent(indent + 1);
                          printf("replacement text: ");
                          printf("%s\n", (*(get_child(cur_node, 2)))-> word-> data);
                          break;
        case FILE_INCLUDE_STATEMENT:
                          display_indent(indent);
                          printf("Included file: ");
                          printf("%s\n", cur_node-> first_child-> word-> data);
                          break;
    }
}