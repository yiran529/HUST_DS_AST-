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
            if(cur_node-> first_child-> first_child) fprintf(fp, "(");
            generate(cur_node-> first_child, fp, 0);
            if(cur_node-> first_child-> first_child) fprintf(fp, ")");
            fprintf(fp, "%s", cur_node-> word.data);
            if((*get_child(cur_node, 2))-> first_child) fprintf(fp, "(");
            generate(*get_child(cur_node, 2), fp, 0);
            if((*get_child(cur_node, 2))-> first_child) fprintf(fp, ")");
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
        fprintf(fp, "return ");
        generate(cur_node-> first_child, fp, 0);
        fprintf(fp, ";\n");
    }
    else if(cur_node-> type == BREAK_STATEMENT) {
        display_indent(indent, fp);
        fprintf(fp, "break;\n");
    }
    else if(cur_node-> type == CONTINUE_STATEMENT) {
        display_indent(indent, fp);
        fprintf(fp, "continue;\n");
    }
    else if(cur_node-> type == EXPRESSION) {
        generate(cur_node-> first_child, fp, indent);
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
            generate(*get_child(cur_node, 2), fp, 0);
        }
    }
    else if(cur_node-> type == EXT_VAR_DEF || cur_node-> type == LOCAL_VAR_DEF) {
        if(cur_node-> type == LOCAL_VAR_DEF) display_indent(indent, fp);
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
    }
    else if(cur_node-> type == FUNC_DEF) {
        display_indent(indent, fp);
        fprintf(fp, "%s", cur_node-> first_child-> word.data);
        fprintf(fp, " ");
        fprintf(fp, "%s", (*get_child(cur_node, 2))-> word.data);
        fprintf(fp, "(");
        generate(*get_child(cur_node, 3), fp, indent);
        fprintf(fp, ")\n");
        if(*get_child(cur_node, 4)) {
            display_indent(indent, fp);
            generate(*get_child(cur_node, 4), fp, indent + 1);
        } else fprintf(fp, ";");
    }
    else if(cur_node-> type == COMPOUND_STATEMENT) {
        display_indent(indent - 1, fp);
        fprintf(fp, "{\n");
        generate((*get_child(cur_node, 1)), fp, indent); // 根据表达式序列生成
        display_indent(indent - 1, fp);
        fprintf(fp, "}\n");
    }
    else if(cur_node-> type == STATEMENT_SEQ) {
        if(cur_node-> first_child-> type == EXPRESSION) display_indent(indent, fp);//单独的表达式前也需要缩进
        generate(cur_node-> first_child, fp, indent);
        if(cur_node-> first_child-> type == EXPRESSION) //单独一个表达式作为一个语句
            fprintf(fp, ";\n");
        generate(*get_child(cur_node, 2), fp, indent);
    }
    else if(cur_node-> type == CONDITIONAL_STATEMENT) {
        generate_conditional_statement(cur_node, fp, indent, false);
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
    else if(cur_node-> type == EXT_DEF_SEQ) {
        generate(cur_node-> first_child, fp, indent);
        generate(*get_child(cur_node, 2), fp, indent);
    }
}// || cur_node-> type == FOR_LOOP || cur_node-> type == CONDITIONAL_STATEMENT

/**
 * 根据以cur_node为根的AST生成条件语句并输入到指定文件中
 * @param cur_node AST的根
 * @param fp 文件指针
 * @param indent 缩进个数
 * @param flag if前需不需要缩进（在else后面的不用）
 */
void generate_conditional_statement(AST_NODE* cur_node, FILE* fp, int indent, bool flag) {
    if(flag == false) display_indent(indent, fp);
    fprintf(fp, "if(");
    generate(cur_node-> first_child, fp, 0);
    fprintf(fp, ")\n");
    generate(*get_child(cur_node, 2), fp, indent + 1);
    if(*get_child(cur_node, 3)) {
        if((*get_child(cur_node, 3))-> type == CONDITIONAL_STATEMENT) {
            display_indent(indent, fp);
            fprintf(fp, "else ");
            generate_conditional_statement(*get_child(cur_node, 3), fp, indent + 1, true);
        } else {
            display_indent(indent, fp);
            fprintf(fp, "else\n");
            generate(*get_child(cur_node, 3), fp, indent + 1);
        }
    }
}