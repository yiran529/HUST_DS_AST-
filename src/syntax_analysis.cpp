#include<string.h>
#include "syntax_analysis.h"
#include "display_AST.h"

extern char token_text[300];

char token_text_copy[300];

AST_NODE* root;

AST_NODE** AST;

int cur_kind, last_kind; // 当前以及上一次读到的单词的种类编号

/**
 * 判断一个种类编号是不是类型关键字
 * @param kind 将要进行判断的单词种类编号
 * @return 如果是，则返回true；否则返回false
 */
bool is_type_specifier(int kind) { //判断是不是类型关键字
    return kind == INT   ||
           kind == SHORT ||
           kind == LONG  ||
           kind == CHAR  ||
           kind == FLOAT ||
           kind == DOUBLE;
}

/**
 * 创建一个AST结点并初始化（该结点为WORD类型）
 * @param node 将要初始化的AST结点（引用指针类型）
 * @param type 结点类型
 * @param kind 单词种类编号
 * @param token_kind0 单词内容
 */
void assign_AST_node(AST_NODE* &node, AST_NODE_TYPE type, TOKEN_KIND kind, char* token_text0) {
    node = (AST_NODE*)malloc(sizeof(AST_NODE));
    node-> type = type;
    node-> word.kind = kind;
    node-> word.data = (char*)malloc(sizeof(char) * (strlen(token_text0) + 1));
    strcpy(node-> word.data, token_text0);
    node-> first_child = node-> next_sibling = NULL;
}

/**
 * 建立以cur_node为根的代表形参序列的AST（左括号已经读在token_text中）
 * @param cur_node 表示形参序列的AST的根的指针引用
 * @param fp_pointer 文件当前读取位置的双重指针
 * @return 如果形参序列没有错，返回true; 否则返回false
 */
bool build_formal_param_seq(AST_NODE* &cur_node, FILE** fp_pointer) {
    assign_AST_node(cur_node, FORMAL_PARAM_SEQ);

    cur_kind = get_token(fp_pointer);

    if(!is_type_specifier(cur_kind)) return false;
    assign_AST_node(cur_node-> first_child, FORMAL_PARAM);
    assign_AST_node(cur_node-> first_child-> first_child, WORD, (TOKEN_KIND)cur_kind, token_text);

    cur_kind = get_token(fp_pointer);
    if(cur_kind != IDENT) return false;
    assign_AST_node(cur_node-> first_child-> first_child-> next_sibling, WORD, (TOKEN_KIND)cur_kind, token_text);

    cur_kind = get_token(fp_pointer);
    if(cur_kind == RP) {
        cur_node = NULL;
        return true;
    } else if(cur_kind != COMMA) return false;
    
    return build_formal_param_seq(cur_node-> first_child-> next_sibling, fp_pointer);
}

/**
 * 建立以cur_node为根的代表函数定义（或声明）的AST（其中函数的返回值类型已经存入cur_node的第一个孩子，函数名
 * 存入token_text_copy, 左括号已经读在token_text中）
 * @param cur_node 表示函数定义（或声明）的AST的根的指针引用
 * @param fp_pointer 文件当前读取位置的双重指针
 * @return 如果函数定义（或声明）没有错误，返回true; 否则返回false
 */
bool build_fun_def(AST_NODE* cur_node, FILE** fp_pointer) {
    assign_AST_node(cur_node-> first_child-> next_sibling, WORD, IDENT, token_text_copy);
    if(build_formal_param_seq(cur_node-> first_child-> next_sibling-> next_sibling, fp_pointer) == false) return false;
    if( == false) return false;
    return true;
}

/**
 * 创建一个AST结点并初始化（该节点不是WORD类型）
 * @param node 将要初始化的AST结点（引用指针类型）
 * @param type 结点类型
 */
void assign_AST_node(AST_NODE* &node, AST_NODE_TYPE type) {
    node = (AST_NODE*)malloc(sizeof(AST_NODE));
    node-> type = type;
    node-> first_child = node-> next_sibling = NULL;
}

/**
 * 以cur_node(引用指针类型)为根构建表示外部变量序列的AST。该定义序列的第一个单词（标识符）已经存储在token_text_copy中。
 * @param cur_node AST根的指针的引用
 * @param fp_pointer 文件当前读取位置的双重指针
 * @return 外部变量序列没有错误，返回true；否则返回false
 */
bool build_ext_var_list(AST_NODE* &cur_node, FILE** fp_pointer) {
    assign_AST_node(cur_node, VAR_SEQ);

    if(cur_kind == SEMI) {
        if(last_kind != IDENT) return false; // 保证分号前面是标识符
        free(cur_node), cur_node = NULL;
        return true;
    } else if(cur_kind == COMMA) {
        if(last_kind != IDENT) return false; // 保证逗号前面是标识符
        last_kind = cur_kind;
        cur_kind = get_token(fp_pointer);
        return build_ext_var_list(cur_node, fp_pointer);
    } else if(cur_kind == IDENT) {
        assign_AST_node(cur_node->first_child, WORD, IDENT, token_text);

        last_kind = cur_kind;
        cur_kind = get_token(fp_pointer);
        return build_ext_var_list(cur_node-> first_child-> next_sibling, fp_pointer);
    } else return false;
}

/**
 * 以cur_node(引用指针类型)为根构建表示外部变量定义的AST。该定义序列的第一个单词（标识符）已经存储
 * 在token_text_copy中, 第二个单词存储在token_text中 。
 * @param cur_node AST根的指针的引用
 * @param fp_pointer 文件当前读取位置的双重指针
 * @return 外部变量定义没有错误，返回true；否则返回false
 */
bool build_ext_var_def(AST_NODE* &cur_node, FILE** fp_pointer) {
    assign_AST_node(cur_node, VAR_SEQ);

    assign_AST_node(cur_node-> first_child, WORD, IDENT, token_text_copy);
    // 这里不需要get_token（根据specification）
    return build_ext_var_list(cur_node-> first_child-> next_sibling, fp_pointer);
}


/**
 * 以cur_node（引用指针类型）为根构建表示外部定义的AST。该定义序列的第一个单词已经被正确地读取。
 * @param cur_node AST根的指针的引用
 * @param fp_pointer 文件当前读取位置的双重指针
 * @return 外部定义没有错误，返回true；否则返回false
 */
bool build_ext_def(AST_NODE* &cur_node, FILE** fp_pointer) {
    assign_AST_node(cur_node, UNKNOWN);

    if(is_type_specifier(cur_kind)) 
        assign_AST_node(cur_node-> first_child, WORD, (TOKEN_KIND)cur_kind, token_text);
    else {
        printf("%d\n", cur_kind);
        return false;
    }

    cur_kind = get_token(fp_pointer);
    if(cur_kind != IDENT) return false;
    strcpy(token_text_copy, token_text);
    last_kind = cur_kind;

    cur_kind = get_token(fp_pointer);
    if(cur_kind == LP) {
        cur_node-> type = (AST_NODE_TYPE)FUNC_DEF;
        return false;//return build_func_def(); TODO
    } else {
        cur_node-> type = (AST_NODE_TYPE)EXT_VAR_DEF;
        return build_ext_var_def(cur_node-> first_child-> next_sibling, fp_pointer);
    }
}



/**
 * 根据所给的源文件构建以cur_root为树根的AST以代表一个定义序列（该源文件已经有一个单词被正确地读取了）
 * @param cur_node 当前所要构建的代表定义序列的AST的树根
 * @param fp_pointer 指向源文件当前读取位置的双重指针
 * @return 如果构建成功，没有错误，则返回true；否则返回false
 */
bool build_ext_def_seq(AST_NODE* &cur_node, FILE** fp_pointer) {
    if(cur_kind == EOF) {
        cur_node = NULL;
        return true;
    }
    assign_AST_node(cur_node, EXT_DEF_SEQ);
    
    if(build_ext_def(cur_node-> first_child, fp_pointer) == false) return false;
    cur_kind = get_token(fp_pointer); // 根据本函数的specification，在该函数被调用前有一个单词已经被正确读取
    return build_ext_def_seq(cur_node-> first_child-> next_sibling, fp_pointer);
    
}


/**
 * 根据所给的源文件构建代表整个程序的AST
 * @param fp_pointer 指向源文件的双重指针
 * @return 如果构建成功，没有错误，则返回true；否则返回false
 */
bool build_program(FILE** fp_pointer) {
    cur_kind = get_token(fp_pointer);
    if(cur_kind == ERROR_TOKEN || cur_kind == EOF) return false;
    assign_AST_node(root, PROGRAM);
    //*AST = root;
    return build_ext_def_seq(root-> first_child, fp_pointer);
}

int main() {
    char path[1000];
    scanf("%s", path);
    FILE *fp = fopen(path, "r");
    build_program(&fp);
    display_AST(root, 0);
    return 0;
}