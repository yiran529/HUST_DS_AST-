#include<stdio.h>
#include<string.h>
#include "display_AST.h"
#include "get_token.h"
#include "syntax_analysis.h"
#include "generate_source_file.h"
extern AST_NODE* root;
int indent;
void display_indent(int indent, FILE* fp) {
    char* one_indent = "    ";
    for(int i = 1; i <= indent; i++) fprintf(fp, "%s", one_indent);
}

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
        if((*get_child(cur_node, 3))-> type == STATEMENT_SEQ && 
            (*get_child(cur_node, 3))-> first_child-> type == CONDITIONAL_STATEMENT) { // else-if
            display_indent(indent, fp);
            fprintf(fp, "else ");
            generate_conditional_statement((*get_child(cur_node, 3))-> first_child, fp, indent, true);
        } else { // else后只是单纯的复合语句或单个语句
            display_indent(indent, fp);
            fprintf(fp, "else\n");
            generate(*get_child(cur_node, 3), fp, indent + 1);
        }
    }
}

/**
 * 根据以cur_node为根的AST生成对应程序并输入到给定文件中
 * @param cur_node AST的根
 * @param fp 文件指针
 * @param indent 缩进个数
 */
void generate(AST_NODE* cur_node, FILE* fp, int indent) {
    if(cur_node == NULL) return;
    else if(cur_node-> type == PROGRAM) generate(cur_node-> first_child, fp, indent);
    else if(cur_node-> type == WORD) {
        if(cur_node-> first_child) { // 是表达式
            bool paren_flag = true; // 需不需要括号
            if(cur_node-> first_child-> type == FUNCTION_CALL ||
            cur_node-> first_child-> word-> kind == IDENT || 
            is_const(cur_node-> first_child-> word-> kind) && 
            cur_node-> first_child-> word-> data[0] != '-') //是负数
                paren_flag = false;
            if(paren_flag) fprintf(fp, "(");
            generate(cur_node-> first_child, fp, 0);
            if(paren_flag) fprintf(fp, ")");

            paren_flag = true;
            if((*get_child(cur_node, 2))-> type == FUNCTION_CALL ||
            (*get_child(cur_node, 2))-> word-> kind == IDENT || 
            is_const((*get_child(cur_node, 2))-> word-> kind) && 
            (*get_child(cur_node, 2))-> word-> data[0] != '-' ) //是负数
                paren_flag = false;
            fprintf(fp, "%s", cur_node-> word-> data);
            if(paren_flag) fprintf(fp, "(");
            generate(*get_child(cur_node, 2), fp, 0);
            if(paren_flag) fprintf(fp, ")");
        } 
        else {
            fprintf(fp, "%s", cur_node-> word-> data);
            if(cur_node-> word-> array_info != NULL) { //不是单纯的标识符，而是数组
                fprintf(fp, "[");
                generate(cur_node-> word-> array_info, fp, 0);
                fprintf(fp, "]");
            }
        }
    }
    else if(cur_node-> type == ACTUAL_PARAM_SEQ) {
        if(cur_node-> first_child == NULL) return; // 实参序列为空
        generate(cur_node-> first_child, fp, 0);
        if(*get_child(cur_node, 2)) {
            fprintf(fp, ",");
            generate(*get_child(cur_node, 2), fp, 0);
        }
    }
    else if(cur_node-> type == FUNCTION_CALL) {
        fprintf(fp, "%s", cur_node-> first_child-> word-> data);
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
        fprintf(fp, "%s", cur_node-> first_child-> word-> data);
        fprintf(fp, " ");
        generate(*get_child(cur_node, 2), fp, indent + 1);
        fprintf(fp, ";\n");
    }
    else if(cur_node-> type == VAR_SEQ) {
        fprintf(fp, "%s", cur_node-> first_child-> word-> data);
        if(*get_child(cur_node, 2)) {
            fprintf(fp, ",");
            generate(*get_child(cur_node, 2), fp, 0);
        }
    }
    else if(cur_node-> type == FUNC_DEF) {
        display_indent(indent, fp);
        fprintf(fp, "%s", cur_node-> first_child-> word-> data);
        fprintf(fp, " ");
        fprintf(fp, "%s", (*get_child(cur_node, 2))-> word-> data);
        fprintf(fp, "(");
        generate(*get_child(cur_node, 3), fp, indent);
        fprintf(fp, ")");
        if(*get_child(cur_node, 4)) {
            fprintf(fp, "\n");
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
        if(cur_node-> first_child-> type != COMPOUND_STATEMENT) 
            generate(cur_node-> first_child, fp, indent);
        else generate(cur_node-> first_child, fp, indent + 1); // 单独的复合语句作为语句时需要注意缩进

        if(cur_node-> first_child-> type == EXPRESSION) //单独一个表达式作为一个语句
            fprintf(fp, ";\n");
        generate(*get_child(cur_node, 2), fp, indent);
    }
    else if(cur_node-> type == CONDITIONAL_STATEMENT) {
        generate_conditional_statement(cur_node, fp, indent, false);
    }
    else if(cur_node-> type == WHILE_LOOP) {
        display_indent(indent, fp);
        fprintf(fp, "while");
        fprintf(fp, "(");
        generate(cur_node-> first_child, fp, 0);
        fprintf(fp, ")\n");
        generate((*get_child(cur_node, 2)), fp, indent + 1);
    } else if(cur_node-> type ==  FOR_LOOP) {
        display_indent(indent, fp);
        fprintf(fp, "for");
        fprintf(fp, "(");
        generate(cur_node-> first_child, fp, 0);
        fprintf(fp, ";");
        generate(*get_child(cur_node, 2), fp, 0);
        fprintf(fp, ";");
        generate(*get_child(cur_node, 3), fp, 0);
        fprintf(fp, ")\n");
        generate((*get_child(cur_node, 4)), fp, indent + 1);
    } else if(cur_node-> type == MACRO_DEFINE_STATEMENT) {
        fprintf(fp, "#define ");
        fprintf(fp, "%s\n", cur_node-> first_child-> word-> data);
    } else if(cur_node-> type == FILE_INCLUDE_STATEMENT) {
        fprintf(fp, "#include ");
        fprintf(fp, "%s\n", cur_node-> first_child-> word-> data);
    }
    else if(cur_node-> type == EXT_DEF_SEQ) {
        generate(cur_node-> first_child, fp, indent);
        generate(*get_child(cur_node, 2), fp, indent);
    } else if(cur_node-> type == DO_WHILE_LOOP) {
        display_indent(indent, fp);
        fprintf(fp, "do\n");
        generate(*get_child(cur_node, 1), fp, indent + 1);
        display_indent(indent, fp);
        fprintf(fp, "while(");
        generate(*get_child(cur_node, 2), fp, 0);
        fprintf(fp, ");\n");
    }
}// || cur_node-> type == FOR_LOOP || cur_node-> type == CONDITIONAL_STATEMENT

