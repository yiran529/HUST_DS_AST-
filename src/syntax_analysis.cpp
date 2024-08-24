#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include "get_token.h"
#include "syntax_analysis.h"
#include "utils.h"

/**
 * 根据所给的源文件构建代表整个程序的AST
 * @param fp_pointer 指向源文件的双重指针
 * @return 如果构建成功，没有错误，则返回true；否则返回false
 */
bool build_program(FILE** fp_pointer) {
    cur_kind = get_token(fp_pointer);
    if(cur_kind == ERROR_TOKEN || cur_kind == EOF) return false;
    root = (AST_NODE*)malloc(sizeof(AST_NODE));
    *AST = root;
    return build_ext_def_seq(root -> first_child, fp_pointer);
}

/**
 * 根据所给的源文件构建定义序列代表的以cur_root为树根的AST（该源文件已经有一个单词被正确地读取了）
 * @param cur_node 当前所要构建的代表定义序列的AST的树根
 * @param fp_pointer 指向源文件当前读取位置的双重指针
 * @return 如果构建成功，没有错误，则返回true；否则返回false
 */
bool build_ext_def_seq(AST_NODE* &cur_node, FILE** fp_pointer) {
    if(cur_kind == EOF) {
        cur_node = NULL;
        return true;
    }
    cur_node = (AST_NODE*)malloc(sizeof(AST_NODE));
    cur_node-> type = EXT_DEF_SEQ, cur_node-> first_child = NULL;
    bool ret_val;
    ret_val = build_ext_def_seq(cur_node-> first_child, fp_pointer);
    ret_val = ret_val && build_ext_def_seq(cur_node-> first_child-> next_sibling, fp_pointer);
    return ret_val;
}

bool build_ext_def_seq(AST_NODE* &cur_node, FILE** fp_pointer) {
    cur_node = (AST_NODE*)malloc(sizeof(AST_NODE));
    cur_node-> type = EXT_DEF;

    if(is_type_specifier(cur_kind)) {
        cur_node-> first_child = (AST_NODE*)malloc(sizeof(AST_NODE));
        cur_node-> first_child-> type = WORD;
        cur_node-> first_child-> word-> kind = cur_kind;
        cur_node-> first_child-> word-> data = (char*)malloc(sizeof(char) * (strlen(token_text) + 1));
        strcpy(cur_node-> first_child-> word-> data, token_text);
        cur_node-> first_child-> first_child = NULL;
    } else return false;

    cur_kind = get_token(fp_pointer);
    if(cur_kind == IDENT) {
        AST_NODE* second_child = cur_node-> first_child-> next_sibling = (AST_NODE*)malloc(sizeof(AST_NODE));
        second_child-> type = WORD;
        second_child-> word-> kind = IDENT;
        
    }
}