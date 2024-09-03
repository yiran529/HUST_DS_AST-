#include<stdio.h>
#include<string.h>
#include "display_AST.h"
#include "get_token.h"
#include "syntax_analysis.h"
extern AST_NODE* root;
int indent;
void display_indent(int indent, FILE* fp) {
    char* one_indent = "    ";
    for(int i = 1; i <= indent; i++) fprintf(fp, "%s", one_indent);
}


void generate(AST_NODE* cur_node, FILE* fp, int indent) {
    if(cur_node == NULL) return;
    else if(cur_node-> type == PROGRAM) generate(cur_node-> first_child, fp, indent);
    else if(cur_node-> type == WORD) {
        if(cur_node-> first_child) { // 是表达式
            display_indent(indent, fp);
            fprintf(fp, "(");
            generate(cur_node-> first_child, fp, 0);
            fprintf(fp, ")");
            fprintf(fp, "%s", cur_node-> word.data);
            fprintf(fp, "(");
            generate(*get_child(cur_node, 2), fp, 0);
            fprintf(fp, ")");
        } 
        else fprintf(fp, "%s", cur_node-> word.data);
    }
    else if(cur_node-> type == ACTUAL_PARAM_SEQ) {
        generate(cur_node-> first_child, fp, 0);
        if(*get_child(cur_node, 2)) {
            fprintf(fp, ",");
            generate(*get_child(cur_node, 2), fp, 0);
        }
    }
    else if(cur_node-> type == FUNCTION_CALL) {
        display_indent(indent, fp);
        fprintf(fp, "%s", cur_node-> first_child);
        fprintf(fp, "(");
        generate(*get_child(cur_node, 2), fp, 0);
        fprintf(fp, ")");
    }
    else if(cur_node-> type == RETURN_STATEMENT) {
        display_indent(indent, fp);
        fprintf(fp, "return \n");
        generate(cur_node-> first_child, fp, 0);
    }
    else if(cur_node-> type == BREAK_STATEMENT) {
        display_indent(indent, fp);
        fprintf(fp, "break;\n");
    }
    else if(cur_node-> type == CONDITIONAL_STATEMENT) {
        display_indent(indent, fp);
        fprintf(fp, "continue;\n");
    }
    else if(cur_node-> type == EXPRESSION) {
        generate(cur_node-> first_child, fp, indent);
    }
    else if(cur_node-> type == EXT_VAR_DEF) {
        generate(cur_node-> first_child, fp, indent);
        generate(*get_child(cur_node, 2), fp, 0);
    } 
    else if(cur_node-> type == NONE_FORMAL_PARAM_SEQ){
        return; 
    }
    else if(cur_node-> type == FORMAL_PARAM) {
        generate(cur_node-> first_child, fp, 0);
        fprintf(fp, " ");
        generate(*get_child(cur_node, 2), fp, 0);
    }
    else if(cur_node-> type == FORMAL_PARAM_SEQ) {
        generate(cur_node-> first_child, fp, 0);
        if(*get_child(cur_node, 2)) {
            fprintf(fp, ",");
            generate(cur_node, fp, 0);
        }
    }
    else if(cur_node-> type == EXT_VAR_DEF || cur_node-> type == LOCAL_VAR_DEF) {
        fprintf(fp, "%s", cur_node-> first_child-> word.data);
        fprintf(fp, " ");
        generate(*get_child(cur_node, 2), fp, indent + 1);
        fprintf(fp, ";\n");
    }
    else if(cur_node-> type == VAR_SEQ) {
        fprintf(fp, "%s", cur_node-> first_child-> word.data);
        if(*get_child(cur_node, 2)) {
            fprintf(fp, ",");
            generate(*get_child(cur_node, 2), fp, 0);
        }
        else fprintf(fp, ";");
    }
    else if(cur_node-> type == FUNC_DEF) {
        display_indent(indent, fp);
        fprintf(fp, "%s", cur_node-> first_child-> word.data);
        fprintf(fp, " ");
        fprintf(fp, "%s", (*get_child(cur_node, 2))-> word.data);
        fprintf(fp, "(");
        generate(*get_child(cur_node, 3), fp, indent);
        fprintf(fp, ")\n");
        display_indent(indent, fp);
        generate(*get_child(cur_node, 4), fp, indent + 1);
    }
    else if(cur_node-> type == COMPOUND_STATEMENT) {
        display_indent(indent, fp);
        fprintf(fp, "{\n");
        generate((*get_child(cur_node, 1)), fp, indent + 1);
        display_indent(indent, fp);
        fprintf(fp, "}\n");
    }
    else if(cur_node-> type == STATEMENT_SEQ) {
        generate(cur_node-> first_child, fp, indent);
    }
    else if(cur_node-> type == CONDITIONAL_STATEMENT) {
        display_indent(indent, fp);
        fprintf(fp, "if(");
        generate(cur_node-> first_child, fp, 0);
        fprintf(fp, ")\n");
        generate(*get_child(cur_node, 2), fp, indent + 1);
        if(*get_child(cur_node, 3)) {
            display_indent(indent, fp);
            fprintf(fp, "else\n");
            generate(*get_child(cur_node, 3), fp, indent + 1);
        }
    }
    else if(cur_node-> type == WHILE_LOOP) {
        fprintf(fp, "while");
        fprintf(fp, "(");
        generate(cur_node-> first_child, fp, 0);
        fprintf(fp, ")");
        generate((*get_child(cur_node, 2)), fp, indent + 1);
    } else if(cur_node-> type ==  FOR_LOOP) {
        fprintf(fp, "for");
        fprintf(fp, "(");
        generate(cur_node-> first_child, fp, 0);
        fprintf(fp, ";");
        generate(*get_child(cur_node, 2), fp, 0);
        fprintf(fp, ";");
        generate(*get_child(cur_node, 3), fp, 0);
        fprintf(fp, ")");
        generate((*get_child(cur_node, 4)), fp, indent + 1);
    }
}// || cur_node-> type == FOR_LOOP || cur_node-> type == CONDITIONAL_STATEMENT