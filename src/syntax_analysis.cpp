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
 * 判断一个种类编号是不是常数
 * @param kind 将要进行判断的单词种类编号
 * @return 如果是，则返回true；否则返回false
 */
bool is_const(int kind) { //判断是不是类型关键字
    return kind == INT_CONST   ||
           // kind == SHORT_CONST || 鉴于short没有特定的后缀，我们不考虑这种情况
           kind == LONG_CONST  ||
           kind == CHAR_CONST  ||
           kind == FLOAT_CONST ||
           kind == DOUBLE_CONST;
           //kind == STRING_CONST; 暂不考虑
}

/**
 * 判断一个种类编号是不是operator
 * @param kind 将要进行判断的单词种类编号
 * @return 如果是，则返回true；否则返回false
 */
bool is_operator(int kind) { //判断是不是类型关键字
    return kind == PLUS      ||
           kind == MINUS     ||
           kind == MULTIPLY  ||
           kind == DEVIDE    ||
           kind == MOD       ||
           kind == LP        ||
           kind == EQ        ||
           kind == GREATER   ||
           kind == LESS      ||
           kind == GREATEREQ ||
           kind == LESSEQ    ||
           kind == AND       ||
           kind == OR        ||
           kind == NOTEQ;
           //还没有考虑赋值
}

/**
 * 根据一个运算符对应单词的种类编号返回其对应的字符串
 * @param kind 单词种类编号
 * @return 字符串指针
 */
char* get_op(TOKEN_KIND kind) {
    char* ret;
    switch(kind) {
        case PLUS:      return "+";
        case MINUS:     return "-";
        case MULTIPLY:  return "*";
        case DEVIDE:    return "/";
        case MOD:       return "%";
        case GREATER:   return ">";
        case LESS:      return "<";
        case GREATEREQ: return ">=";
        case LESSEQ:    return "<=";
        case EQ:        return "==";
        case NOTEQ:     return "!=";
        case AND:       return "&&";
        case OR:        return "||";
        default:        return "ERROR at get_op!";
    }
}

/**
 * 创建一个AST结点并初始化（该结点为WORD类型）
 * @param node 将要初始化的AST结点（引用指针类型）
 * @param type 结点类型
 * @param kind 单词种类编号
 * @param token_text0 单词内容
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
 * 创建一个AST结点并初始化（该节点不是WORD类型）
 * @param node 将要初始化的AST结点（引用指针类型）
 * @param type 结点类型
 */
void assign_AST_node(AST_NODE* &node, AST_NODE_TYPE type) {
    node = (AST_NODE*)malloc(sizeof(AST_NODE));
    node-> type = type;
    node-> first_child = node-> next_sibling = NULL;
}

bool build_expression(AST_NODE* &cur_node, FILE** fp_pointer, TOKEN_KIND end_sym);
bool build_compound_statement(AST_NODE* &cur_node, FILE** fp_pointer);
bool build_statement(AST_NODE* &cur_node, FILE** fp_pointer);

/***********************************************************************************************************/

/**
 * 创建一个AST结点表示条件语句（if已经被读取）。若该函数被正确调用，下一部分的第一个单词也会被读取。
 * @param cur_node AST根的指针的引用
 * @param fp_pointer 文件当前读取位置的双重指针
 * @return 条件语句没有错误，返回true；否则返回false
 */
bool build_conditional_statement(AST_NODE* &cur_node, FILE** fp_pointer) {
    assign_AST_node(cur_node, CONDITIONAL_STATEMENT);

    cur_kind = get_token(fp_pointer);
    if(cur_kind != LP) return false;
    cur_kind = get_token(fp_pointer);
    if(build_expression(cur_node-> first_child, fp_pointer, RP) == false) return false;

    cur_kind = get_token(fp_pointer);
    if(cur_kind == LC) {
        cur_kind = get_token(fp_pointer);
        if(build_compound_statement(cur_node-> first_child-> next_sibling, fp_pointer) == false) return false;
    } else {
        assign_AST_node(cur_node-> first_child-> next_sibling, STATEMENT_SEQ); // 为了复合语句的情况统一
        if(build_statement(cur_node-> first_child-> next_sibling-> first_child, fp_pointer) == false) return false;
    }

    if(cur_kind != ELSE) return true;

    cur_kind = get_token(fp_pointer);
    if(cur_kind == LC) 
        return build_compound_statement(cur_node-> first_child-> next_sibling-> next_sibling, fp_pointer);
    else if(cur_kind == IF) 
        return build_conditional_statement(cur_node-> first_child-> next_sibling-> next_sibling, fp_pointer);
    else {
        assign_AST_node(cur_node-> first_child-> next_sibling-> next_sibling, STATEMENT_SEQ); // 为了复合语句的情况统一
        return build_statement(cur_node-> first_child-> next_sibling-> next_sibling-> first_child, fp_pointer);
    }
}

int op_cmp[200][200] = {
    {1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1},
    {1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1},
    {0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1},
    {0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1}
};
// 0 表示小于

int op_num[100]; // 给每个可能在表达式中出现的表示运算的单词编号，便于表达式分析
/**
 * 给op_num初始化，索引为表达式中可能出现的表示运算符的单词，值为其对应的编号
 */
void assign_op_num() {
    op_num[PLUS] = 0;
    op_num[MINUS] = 1;
    op_num[MULTIPLY] = 2;
    op_num[DEVIDE] = 3;
    op_num[MOD] = 4;
    op_num[LP] = 5;
    op_num[GREATER] = op_num[LESS] = op_num[GREATEREQ] = op_num[LESSEQ] = 6;
    op_num[EQ] = op_num[NOTEQ] = 7;
    op_num[HASH] = 8;
    op_num[AND] = 9;
    op_num[OR] = 10;
}

/**
 * 以cur_node(引用指针类型)为根构建表示表达式的AST。该表达式的第一个单词（标识符）已经存储在token_text中。
 * 函数调用后，一直读取至遇见正确的end_sym。
 * @param cur_node AST根的指针的引用
 * @param fp_pointer 文件当前读取位置的双重指针
 * @param end_sym 标志表达式结束的单词。且要么是分号，要么是右括号。
 * @return 表达式没有错误，返回true；否则返回false
 */
bool build_expression(AST_NODE* &cur_node, FILE** fp_pointer, TOKEN_KIND end_sym) {
    assign_op_num();
    assign_AST_node(cur_node, EXPRESSION);
    int       op[300]; 
    AST_NODE* opn[300]; 
    int op_index = 0, opn_index = 0;
    op[op_index++] = HASH; // 用-1表示栈底元素


    while(1) {
        if(cur_kind == ERROR_TOKEN) return false;
        if(cur_kind == SEMI && end_sym == SEMI) break;
        if(cur_kind == IDENT || is_const(cur_kind)) {
            if(opn_index > 0 && !is_operator(last_kind))  return false;
            assign_AST_node(opn[opn_index++], WORD, (TOKEN_KIND)cur_kind, token_text);
        } else if(is_operator(cur_kind)) {
            if(cur_kind != LP && last_kind != IDENT && !is_const(last_kind) && last_kind != RP) return false;
            if(cur_kind == LP && !is_operator(last_kind) && (op_index + opn_index > 1)) return false;
            switch(op_cmp[op_num[cur_kind]][op_num[op[op_index - 1]]]) {
                case 1: //大于
                        op[op_index++] = cur_kind;  
                        break;
                case 0: //小于
                        while(op_cmp[op_num[cur_kind]][op_num[op[op_index - 1]]] == 0 && op_index > 1) {
                            AST_NODE* new_node; 
                            assign_AST_node(new_node, WORD, (TOKEN_KIND)op[op_index - 1], get_op((TOKEN_KIND)op[op_index - 1]));
                            op_index--;
                            new_node-> first_child = opn[opn_index - 2]; // bad smell
                            new_node-> first_child-> next_sibling = opn[opn_index - 1];
                            opn_index--;
                            opn[opn_index - 1] = new_node;
                        }
                        op[op_index++] = cur_kind;
                        break;
                // case 2: break; 等于
            }
        } else if(cur_kind == RP) {
            while(op[op_index - 1] != LP && op_index > 1) {
                AST_NODE* new_node; 
                assign_AST_node(new_node, WORD, (TOKEN_KIND)op[opn_index - 1], get_op((TOKEN_KIND)op[op_index - 1]));
                op_index--;
                new_node-> first_child = opn[opn_index - 2]; // bad smell
                new_node-> first_child-> next_sibling = opn[opn_index - 1];
                opn_index--;
                opn[opn_index - 1] = new_node;
            }
            if(op_index == 1 && opn_index == 1 && end_sym == RP) {
                cur_node-> first_child = opn[0];
                return true;
            }
            else if(op_index == 1) return false;
            op_index--; // 将左括号出栈
        } else return false;
        last_kind = cur_kind;
        strcpy(token_text_copy, token_text);
        cur_kind = get_token(fp_pointer);
    }
    //最后将剩下的进行计算, 只针对结尾是分号的情况
    while(op_index > 1 && op[op_index - 1] != LP) {
        AST_NODE* new_node; 
        assign_AST_node(new_node, WORD, (TOKEN_KIND)op[opn_index - 1], get_op((TOKEN_KIND)op[op_index - 1]));
        op_index--;
        new_node-> first_child = opn[opn_index - 2]; // bad smell
        new_node-> first_child-> next_sibling = opn[opn_index - 1];
        opn_index--;
        opn[opn_index - 1] = new_node;
    }
    if(op[op_index - 1] == LP) return false;
    cur_node->first_child = opn[0]; 
    return true;
}

/**
 * 以cur_node(引用指针类型)为根构建表示变量序列的AST。该定义序列的第一个单词（标识符）已经存储在token_text中。
 * 函数调用后，下一部分的第一个单词也会被读取。
 * @param cur_node AST根的指针的引用
 * @param fp_pointer 文件当前读取位置的双重指针
 * @return 变量序列没有错误，返回true；否则返回false
 */
bool build_var_list(AST_NODE* &cur_node, FILE** fp_pointer) {
    assign_AST_node(cur_node, VAR_SEQ);

    if(cur_kind == SEMI) {
        if(last_kind != IDENT) return false; // 保证分号前面是标识符
        free(cur_node), cur_node = NULL;
        cur_kind = get_token(fp_pointer);
        return true;
    } else if(cur_kind == COMMA) {
        if(last_kind != IDENT) return false; // 保证逗号前面是标识符
        last_kind = cur_kind;
        cur_kind = get_token(fp_pointer);
        return build_var_list(cur_node, fp_pointer);
    } else if(cur_kind == IDENT) {
        assign_AST_node(cur_node->first_child, WORD, IDENT, token_text);
        last_kind = cur_kind;
        cur_kind = get_token(fp_pointer);
        return build_var_list(cur_node-> first_child-> next_sibling, fp_pointer);
    } else return false;
}

/**
 * 建立以cur_node为根的代表语句的AST（第一个单词已经读在token_text中）。如果该函数被正确调用，
 * 那么下一部分的第一个单词也会被读取。
 * 另外，注意这里的“语句”对原来的进行了一定拓展。
 * @param cur_node 表示语句的AST的根的指针引用
 * @param fp_pointer 文件当前读取位置的双重指针
 * @return 如果语句没有错，返回true; 否则返回false
 */
bool build_statement(AST_NODE* &cur_node, FILE** fp_pointer) {
    if(cur_kind == ERROR_TOKEN || cur_kind == EOF) return false;
    else if(is_type_specifier(cur_kind)) { // 是局部变量定义
        assign_AST_node(cur_node, LOCAL_VAR_DEF);
        assign_AST_node(cur_node-> first_child, WORD, (TOKEN_KIND)cur_kind, token_text);

        cur_kind = get_token(fp_pointer);
        return build_var_list(cur_node-> first_child-> next_sibling, fp_pointer);
    } else if(cur_kind == IDENT || is_const(cur_kind) || cur_kind == LP) { // 是表达式
        bool ret = build_expression(cur_node, fp_pointer, SEMI);
        cur_kind = get_token(fp_pointer);
        return ret;
    } else if(cur_kind == IF) {
        return build_conditional_statement(cur_node, fp_pointer);
    }
    return false;
}

/**
 * 建立以cur_node为根的代表语句序列的AST（第一个单词已经读在token_text中）。
 * 如果语句序列正确，那么下一部分第一个单词也会被读取。
 * @param cur_node 表示语句序列的AST的根的指针引用
 * @param fp_pointer 文件当前读取位置的双重指针
 * @return 如果语句序列没有错，返回true; 否则返回false
 */
bool build_statement_seq(AST_NODE* &cur_node, FILE** fp_pointer) {
    if(cur_kind == RC) {
        cur_kind = get_token(fp_pointer);
        return true;
    }

    assign_AST_node(cur_node, STATEMENT_SEQ);
    if(build_statement(cur_node-> first_child, fp_pointer) == false) return false;
    return build_statement_seq(cur_node-> first_child-> next_sibling, fp_pointer);
}

/**
 * 建立以cur_node为根的代表复合语句的AST（第一个单词，即左括号后一个单词，已经读在token_text中）。调用该函数后，
 * 如果符合语句定义正确，那么下一部分的第一个单词(右括号后一个单词)也会被读取。
 * @param cur_node 表示复合语句的AST的根的指针引用
 * @param fp_pointer 文件当前读取位置的双重指针
 * @return 如果复合语句没有错，返回true; 否则返回false
 */
bool build_compound_statement(AST_NODE* &cur_node, FILE** fp_pointer) {
    assign_AST_node(cur_node, COMPOUND_STATEMENT);
    
    if(build_statement_seq(cur_node-> first_child, fp_pointer) == false) return false;
    //前面的build_statement_seq的调用保证了本函数的post-condition会被满足
    return true;
}

/**
 * 建立以cur_node为根的代表形参序列的AST（左括号已经读在token_text中）。该函数被调用后，
 * 下一部分的第一个单词BU会被读取。
 * @param cur_node 表示形参序列的AST的根的指针引用
 * @param fp_pointer 文件当前读取位置的双重指针
 * @return 如果形参序列没有错，返回true; 否则返回false
 */
bool build_formal_param_seq(AST_NODE* &cur_node, FILE** fp_pointer) {
    last_kind = cur_kind;
    cur_kind = get_token(fp_pointer);
    if(last_kind == LP && cur_kind == RP) { //没有形参的情况需要特殊判断
        assign_AST_node(cur_node, NONE_FORMAL_PARAM_SEQ);
        return true;
    }

    if(!is_type_specifier(cur_kind)) return false;
    assign_AST_node(cur_node, FORMAL_PARAM_SEQ);
    assign_AST_node(cur_node-> first_child, FORMAL_PARAM);
    assign_AST_node(cur_node-> first_child-> first_child, WORD, (TOKEN_KIND)cur_kind, token_text);

    cur_kind = get_token(fp_pointer);
    if(cur_kind != IDENT) return false;
    assign_AST_node(cur_node-> first_child-> first_child-> next_sibling, WORD, (TOKEN_KIND)cur_kind, token_text);

    cur_kind = get_token(fp_pointer);
    if(cur_kind == RP) return true;
    else if(cur_kind != COMMA) return false;

    return build_formal_param_seq(cur_node-> first_child-> next_sibling, fp_pointer);
}

/**
 * 建立以cur_node为根的代表函数定义（或声明）的AST（其中函数的返回值类型已经存入cur_node的第一个孩子，函数名
 * 存入token_text_copy, 左括号已经读在token_text中）。该函数被调用后，下一部分的第一个单词也会被读取。
 * @param cur_node 表示函数定义（或声明）的AST的根的指针引用
 * @param fp_pointer 文件当前读取位置的双重指针
 * @return 如果函数定义（或声明）没有错误，返回true; 否则返回false
 */
bool build_fun_def(AST_NODE* cur_node, FILE** fp_pointer) {
    assign_AST_node(cur_node-> first_child-> next_sibling, WORD, IDENT, token_text_copy);
    if(build_formal_param_seq(cur_node-> first_child-> next_sibling-> next_sibling, fp_pointer) == false) return false;
    cur_kind = get_token(fp_pointer);
    if(cur_kind == SEMI) {
        cur_kind = get_token(fp_pointer);
        return true; 
    } else if (cur_kind == LC) {
        cur_kind = get_token(fp_pointer);
        return build_compound_statement(cur_node-> first_child-> next_sibling-> next_sibling-> next_sibling, fp_pointer);
    } else return false;
    //return ...;
}



/**
 * 以cur_node(引用指针类型)为根构建表示变量定义的AST。该定义序列的第一个单词（标识符）已经存储
 * 在token_text_copy中, 第二个单词存储在token_text中 。
 * @param cur_node AST根的指针的引用
 * @param fp_pointer 文件当前读取位置的双重指针
 * @return 外部变量定义没有错误，返回true；否则返回false
 */
bool build_var_def(AST_NODE* &cur_node, FILE** fp_pointer) {
    assign_AST_node(cur_node, VAR_SEQ);

    assign_AST_node(cur_node-> first_child, WORD, IDENT, token_text_copy);
    // 这里不需要get_token（根据specification）
    return build_var_list(cur_node-> first_child-> next_sibling, fp_pointer);
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
        return build_fun_def(cur_node, fp_pointer);
    } else {
        cur_node-> type = (AST_NODE_TYPE)EXT_VAR_DEF;
        return build_var_def(cur_node-> first_child-> next_sibling, fp_pointer);
    }
}



/**
 * 根据所给的源文件构建以cur_root为树根的AST以代表一个定义序列（该源文件已经有一个单词被正确地读取了）。
 * 函数被调用后，下一部分的第一个单词也会被调用。
 * @param cur_node 当前所要构建的代表定义序列的AST的树根
 * @param fp_pointer 指向源文件当前读取位置的双重指针
 * @return 如果构建成功，没有错误，则返回true；否则返回false
 */
bool build_ext_def_seq(AST_NODE* &cur_node, FILE** fp_pointer) {
    if(cur_kind == EOF) { cur_node = NULL; return true; }
    assign_AST_node(cur_node, EXT_DEF_SEQ);
    
    if(build_ext_def(cur_node-> first_child, fp_pointer) == false) return false;
    //cur_kind = get_token(fp_pointer); // 根据本函数的specification
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
;
    if(build_program(&fp)) 
    display_AST(root, 0);
    else  printf("ERROR!");
    return 0;
}