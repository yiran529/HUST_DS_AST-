#include<string.h>
#include "syntax_analysis.h"
#include "display_AST.h"

extern char token_text[300];

extern int col, row;

char token_text_copy[300];

char error_message[300]; // 用以存储报错信息

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
           kind == ASSIGN    ||
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
        case ASSIGN:    return "=";
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

/**
 * 获取node的第x个孩子的双重指针
 * @param node 父节点
 * @param x 所需要获取的孩子是父节点的第几个孩子
 * @return 所获取的孩子的双重指针
 */
struct AST_NODE** get_child(AST_NODE* node, int x) {
    AST_NODE** cur_node =  &node-> first_child;
    for(int i = 2; i <= x; i++) cur_node = &((*cur_node)-> next_sibling);
    return cur_node; 
} 

bool build_expression(AST_NODE* &cur_node, FILE** fp_pointer, TOKEN_KIND end_sym);
bool build_compound_statement(AST_NODE* &cur_node, FILE** fp_pointer);
bool build_statement(AST_NODE* &cur_node, FILE** fp_pointer);

/***********************************************************************************************************/
/**
 * 以cur_node(引用指针类型)为根构建表示实参序列的AST。左括号已经存储在token_text中。函数正确调用后，右括号会被读取。
 * cur_node的第一个孩子表示第一个实参（表达式），第二个孩子表示后面的实参序列。
 * @param cur_node AST根的指针的引用
 * @param fp_pointer 文件当前读取位置的双重指针
 * @return 实参序列没有错误，返回true；否则返回false
 */
bool build_actual_param(AST_NODE* &cur_node, FILE** fp_pointer){
    if(cur_kind == RP) return true;
    assign_AST_node(cur_node, ACTUAL_PARAM_SEQ);
    
    last_kind = cur_kind;
    cur_kind = get_token(fp_pointer);
    if(cur_kind == RP && last_kind == LP) return true; // 对没有实参的情况进行特判，but bad smell
    if(build_expression(cur_node-> first_child, fp_pointer, COMMA_OR_RP) == false) return false;
    return build_actual_param(*get_child(cur_node, 2), fp_pointer);
}

/**
 * 以cur_node(引用指针类型)为根构建表示函数调用的AST。左括号已经存储在token_text中。函数正确调用后，右括号会被读取。
 * cur_node第一个孩子表示函数名，第二个孩子表示实参序列。
 * @param cur_node AST根的指针的引用
 * @param fp_pointer 文件当前读取位置的双重指针
 * @return 函数调用没有错误，返回true；否则返回false
 */
bool build_function_call(AST_NODE* &cur_node, FILE** fp_pointer){
    assign_AST_node(cur_node, FUNCTION_CALL);
    assign_AST_node(cur_node-> first_child, WORD, IDENT, token_text_copy);
    last_kind = cur_kind; //用以进行特判
    return build_actual_param(*get_child(cur_node, 2), fp_pointer);
}

/**
 * 以cur_node(引用指针类型)为根构建表示for循环的AST。for已经存储在token_text中。函数正确调用后，下一部分的单词会被读取。
 * 其中父节点的第一个孩子为代表for第一条语句的表达式，第二个孩子为代表for第二条语句，第三个孩子代表for第三条语句，第四个
 * 孩子代表for语句后面的复合语句或单个语句（视为仅含一个语句的语句序列）。
 * @param cur_node AST根的指针的引用
 * @param fp_pointer 文件当前读取位置的双重指针
 * @return for循环没有错误，返回true；否则返回false
 */
bool build_for_statement(AST_NODE* &cur_node, FILE** fp_pointer) {
    cur_kind = get_token(fp_pointer);
    if(cur_kind != LP) {
        strcpy(error_message, "\"for\" should be followed by '('.");
        return false;
    }

    assign_AST_node(cur_node, FOR_LOOP);
    cur_kind = get_token(fp_pointer);
    if(build_expression(cur_node-> first_child, fp_pointer, SEMI) == false) return false;
    cur_kind = get_token(fp_pointer);
    if(build_expression(*get_child(cur_node, 2), fp_pointer, SEMI) == false) return false;
    cur_kind = get_token(fp_pointer);
    if(cur_kind != RP) {// 判断第三个表达式是否为空（允许为空）
        if(build_expression(*get_child(cur_node, 3), fp_pointer, RP) == false) return false;
    }
    else {
        assign_AST_node(*get_child(cur_node, 3), EXPRESSION);
    }

    cur_kind = get_token(fp_pointer);
    if(cur_kind == LC) {
        cur_kind = get_token(fp_pointer);
        if(build_compound_statement(*get_child(cur_node, 4), fp_pointer) == false) return false;
    }
    else {
        assign_AST_node(*get_child(cur_node, 4), STATEMENT_SEQ);
        if(build_statement((*get_child(cur_node, 4))-> first_child, fp_pointer) == false) return false;
    }
    return true;
}

/**
 * 以cur_node(引用指针类型)为根构建表示while循环的AST。while后的第一个单词（如果是正确的话，应当是左括号）已经存储在token_text中。
 * 函数调用后，下一部分的单词会被读取。
 * 其中父节点的第一个孩子为代表循环继续的条件的表达式，第二个孩子为代表循环主主体的语句复合语句或语句序列（只有一个语句）。
 * @param cur_node AST根的指针的引用
 * @param fp_pointer 文件当前读取位置的双重指针
 * @return while循环没有错误，返回true；否则返回false
 */
bool build_while_statement(AST_NODE* &cur_node, FILE** fp_pointer) {
    if(cur_kind != LP) {
        strcpy(error_message, "\"while\" should be followed by '('.");
        return false;
    }
    assign_AST_node(cur_node, WHILE_LOOP);

    cur_kind = get_token(fp_pointer);
    if(build_expression(cur_node-> first_child, fp_pointer, RP) == false) return false;
    
    cur_kind = get_token(fp_pointer);
    if(cur_kind == LC) {
        cur_kind = get_token(fp_pointer);
        if(build_compound_statement(*get_child(cur_node, 2), fp_pointer) == false) return false;
    } else {
        assign_AST_node(*get_child(cur_node, 2), STATEMENT_SEQ);
        return build_statement((*get_child(cur_node, 2))-> first_child, fp_pointer);
    } 
    return true;
}

/**
 * 以cur_node(引用指针类型)为根构建表示return语句的AST。该表达式的第一个单词（标识符）已经存储在token_text中。
 * 函数调用后，下一部分的单词BU会被读取。
 * @param cur_node AST根的指针的引用
 * @param fp_pointer 文件当前读取位置的双重指针
 * @return return语句没有错误，返回true；否则返回false
 */
bool build_return_statement(AST_NODE*& cur_node, FILE** fp_pointer) {
    assign_AST_node(cur_node, RETURN_STATEMENT);
    cur_kind = get_token(fp_pointer);
    return build_expression(cur_node-> first_child, fp_pointer, SEMI);
}

/**
 * 创建一个AST结点表示条件语句（if已经被读取）。若该函数被正确调用，下一部分的第一个单词也会被读取。
 * 表示条件语句的第一个子节点代表表示条件的表达式，第二个子节点代表表示子句的表达式，第三个子节点
 * （如果存在）表示else后面的复合语句或语句序列。
 * @param cur_node AST根的指针的引用
 * @param fp_pointer 文件当前读取位置的双重指针
 * @return 条件语句没有错误，返回true；否则返回false
 */
bool build_conditional_statement(AST_NODE* &cur_node, FILE** fp_pointer) {
    assign_AST_node(cur_node, CONDITIONAL_STATEMENT);

    cur_kind = get_token(fp_pointer);
    if(cur_kind != LP) {
        strcpy(error_message, "\"if\" should be followed by '('.");
        return false;
    }
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
    if(cur_kind == LC) {
        cur_kind = get_token(fp_pointer);
        return build_compound_statement(*get_child(cur_node, 3), fp_pointer);
    }
    else if(cur_kind == IF) {
        assign_AST_node(*get_child(cur_node, 3), STATEMENT_SEQ); // 让后面两种情况统一。总之，else后面要么是复合语句，要么是语句序列。
        return build_conditional_statement((*get_child(cur_node, 3))-> first_child, fp_pointer);
    }
    else { // else后面只有单纯的语句，即既不是符合语句，也不是if-else
        assign_AST_node(*get_child(cur_node, 3), STATEMENT_SEQ); 
        return build_statement((*get_child(cur_node, 3))-> first_child, fp_pointer);
    }
}

int op_cmp[200][200] = {
    {1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1},
    {0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1},
    {0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1},
    {0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1}
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
    op_num[ASSIGN] = 11;
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
    if(cur_kind == SEMI) return true; // 空语句的情况特殊判断
    int       op[300]; 
    AST_NODE* opn[300]; 
    opn[0] = NULL; // 解决空语句的问题
    int op_index = 0, opn_index = 0;
    op[op_index++] = HASH; // 用-1表示栈底元素


    while(1) {
        if(cur_kind == ERROR_TOKEN) {
            strcpy(error_message, "Error token.");
            return false;
        } else if(cur_kind == LP && last_kind == IDENT) { // 处理函数调用的情况
            if(build_function_call(opn[opn_index - 1], fp_pointer) == false) return false;
        } else if(cur_kind == SEMI && end_sym == SEMI) break;
          else if(cur_kind == COMMA && end_sym == COMMA_OR_RP) break;
          else if(cur_kind == IDENT || is_const(cur_kind)) {
            if(opn_index > 0 && !is_operator(last_kind)) {
                strcpy(error_message, "An operand should follow an operator.");
                return false;
            }
            assign_AST_node(opn[opn_index++], WORD, (TOKEN_KIND)cur_kind, token_text);
        } else if(is_operator(cur_kind)) {
            if(cur_kind == ASSIGN) { // 赋值符号情况特殊，需要特别判断
                if(last_kind != IDENT) {
                    strcpy(error_message, "Wrong usage of '='.");
                    return false;
                }
            }
            if(cur_kind != LP && last_kind != IDENT && !is_const(last_kind) && last_kind != RP) {
                strcpy(error_message, "Operand should be followed by operator.");
                return false;
            }
            if(cur_kind == LP && !is_operator(last_kind) && (op_index + opn_index > 1)) {
                strcpy(error_message, "Wrong usage of '('.");
                return false;
            }
            switch(op_cmp[op_num[cur_kind]][op_num[op[op_index - 1]]]) {
                case 1: //大于
                        op[op_index++] = cur_kind;  
                        break;
                case 0: //小于
                        if(cur_kind == ASSIGN) {
                            strcpy(error_message, "Wrong usage of '='.");
                            return false;
                        }
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
            if(op_index == 1 && opn_index == 1 && (end_sym == RP || end_sym == COMMA_OR_RP)) { 
                cur_node-> first_child = opn[0];
                return true;
            }
            else if(op_index == 1) {
                strcpy(error_message, "Unexpected ')' here.");
                return false;
            }
            op_index--; // 将左括号出栈
        } else {
            strcpy(error_message, "Unexpiiected token here.");
            return false;
        }
        last_kind = cur_kind;
        strcpy(token_text_copy, token_text);
        cur_kind = get_token(fp_pointer);
    }
    if(opn_index < op_index) { // 结尾多了一个运算符，或括号没有闭合
        strcpy(error_message, "Unexpected token here."); 
        return false;
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
    if(op[op_index - 1] == LP) {
        strcpy(error_message, "'(' not closed.");
        return false;
    }
    cur_node-> first_child = opn[0]; 
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
        if(last_kind != IDENT) {// 保证分号前面是标识符
            strcpy(error_message, "Wrong definition of the variables.");
            return false; 
        }
        free(cur_node), cur_node = NULL;
        cur_kind = get_token(fp_pointer);
        return true;
    } else if(cur_kind == COMMA) {
        if(last_kind != IDENT) {// 保证逗号前面是标识符
            strcpy(error_message, "Wrong definition of the variables");
            return false; 
        }
        last_kind = cur_kind;
        cur_kind = get_token(fp_pointer);
        return build_var_list(cur_node, fp_pointer);
    } else if(cur_kind == IDENT) {
        assign_AST_node(cur_node->first_child, WORD, IDENT, token_text);
        last_kind = cur_kind;
        cur_kind = get_token(fp_pointer);
        return build_var_list(cur_node-> first_child-> next_sibling, fp_pointer);
    } else {
        strcpy(error_message, "Wrong definition of the variables.");
        return false;
    }
}


/*************************************************************************************************************/
/**
 * 建立以cur_node为根的代表语句的AST（第一个单词已经读在token_text中）。如果该函数被正确调用，
 * 那么下一部分的第一个单词(一般来说，是分号后面一个单词）也会被读取。
 * 另外，注意这里的“语句”对原来的进行了一定拓展。
 * @param cur_node 表示语句的AST的根的指针引用
 * @param fp_pointer 文件当前读取位置的双重指针
 * @return 如果语句没有错，返回true; 否则返回false
 */
bool build_statement(AST_NODE* &cur_node, FILE** fp_pointer) {
    if(cur_kind == ERROR_TOKEN || cur_kind == EOF) {
        if(cur_kind == ERROR_TOKEN) strcpy(error_message, "Error token.");
        else strcpy(error_message, "Expected '}' here.");
        return false;
    } else if(is_type_specifier(cur_kind)) { // 是局部变量定义
        assign_AST_node(cur_node, LOCAL_VAR_DEF);
        assign_AST_node(cur_node-> first_child, WORD, (TOKEN_KIND)cur_kind, token_text);
        cur_kind = get_token(fp_pointer);
        return build_var_list(cur_node-> first_child-> next_sibling, fp_pointer);
    } else if(cur_kind == IDENT || is_const(cur_kind) || cur_kind == LP || cur_kind == SEMI) { // 是表达式(可以只含一个分号)
        if(build_expression(cur_node, fp_pointer, SEMI) == false) return false;
        cur_kind = get_token(fp_pointer);
        return true;
    } else if(cur_kind == IF) {
        return build_conditional_statement(cur_node, fp_pointer);
    } else if(cur_kind == RETURN) {
        if(build_return_statement(cur_node, fp_pointer) == false) return false;
        cur_kind = get_token(fp_pointer);
        return true;
    } else if(cur_kind == WHILE) {
        cur_kind = get_token(fp_pointer);
        if(build_while_statement(cur_node, fp_pointer) == false) return false;
        return true;
    } else if(cur_kind == FOR) {
        if(build_for_statement(cur_node, fp_pointer) == false) return false;
        return true;
    } else if(cur_kind == BREAK) {
        cur_kind = get_token(fp_pointer);
        if(cur_kind != SEMI) {
            strcpy(error_message, "\"break\" should be followed by ';'.");
            return false;
        }
        assign_AST_node(cur_node, BREAK_STATEMENT);
        cur_kind = get_token(fp_pointer);
        return true;
    } else if(cur_kind == CONTINUE) {
        cur_kind = get_token(fp_pointer);
        if(cur_kind != SEMI) {
            strcpy(error_message, "\"continue\" should be followed by ';'.");
            return false;
        }
        assign_AST_node(cur_node, CONTINUE_STATEMENT);
        cur_kind = get_token(fp_pointer);
        return true;
    } else if(cur_kind == LC) {
        cur_kind = get_token(fp_pointer);
        return build_compound_statement(cur_node, fp_pointer);
    }
    strcpy(error_message, "Unexpected token here.");
    return false;
}

/**
 * 建立以cur_node为根的代表语句序列的AST（第一个单词已经读在token_text中）。
 * 如果语句序列正确，那么下一部分第一个单词也会被读取。 注意，只能用在复合语句中。
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
    
    if(cur_kind == RC) { // 处理复合语句完全为空的情况
        strcpy(error_message, "Compound statement should not be empty.");
        return false; 
    }

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

    if(!is_type_specifier(cur_kind)) {
        strcpy(error_message, "Expected a type specifier here.");
        return false;
    }
    assign_AST_node(cur_node, FORMAL_PARAM_SEQ);
    assign_AST_node(cur_node-> first_child, FORMAL_PARAM);
    assign_AST_node(cur_node-> first_child-> first_child, WORD, (TOKEN_KIND)cur_kind, token_text);

    cur_kind = get_token(fp_pointer);
    if(cur_kind != IDENT) {
        strcpy(error_message, "Expected an identity here.");
        return false;
    }
    assign_AST_node(cur_node-> first_child-> first_child-> next_sibling, WORD, (TOKEN_KIND)cur_kind, token_text);

    cur_kind = get_token(fp_pointer);
    if(cur_kind == RP) return true;
    else if(cur_kind != COMMA) {
        strcpy(error_message, "Expected comma here.");
        return false;
    }

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
    if(cur_kind == SEMI) { // 函数声明
        cur_kind = get_token(fp_pointer);
        return true; 
    } else if (cur_kind == LC) { // 函数定义
        cur_kind = get_token(fp_pointer);
        return build_compound_statement(cur_node-> first_child-> next_sibling-> next_sibling-> next_sibling, fp_pointer);
    } else {
        strcpy(error_message, "Unexpected token here.");
        return false;
    }
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

    int type = cur_kind;
    if(is_type_specifier(cur_kind) || cur_kind == VOID) // 特殊判断函数返回值为空的情况
        assign_AST_node(cur_node-> first_child, WORD, (TOKEN_KIND)cur_kind, token_text);
    else {
        strcpy(error_message, "Expected type specifier here.");
        return false;
    }

    cur_kind = get_token(fp_pointer);
    if(cur_kind != IDENT) {
        strcpy(error_message, "Expected identity here.");
        return false;
    }
    strcpy(token_text_copy, token_text);
    last_kind = cur_kind;

    cur_kind = get_token(fp_pointer);
    if(cur_kind == LP) {
        cur_node-> type = (AST_NODE_TYPE)FUNC_DEF;
        return build_fun_def(cur_node, fp_pointer);
    } else {
        if(type == VOID) { // 变量定义类型不能为void，这个情况需要特殊判断
            strcpy(error_message, "The type of variables cannot be \"void\".");
            return false; 
        }
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
    if(cur_kind == ERROR_TOKEN || cur_kind == EOF) {
        if(cur_kind == ERROR_TOKEN) strcpy(error_message, "Error token.");
        else                        strcpy(error_message, "The file is empty");
        return false;
    }
    assign_AST_node(root, PROGRAM);
    //*AST = root;
    return build_ext_def_seq(root-> first_child, fp_pointer);
}
