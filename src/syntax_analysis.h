#ifndef SYNTAX_ANALYSIS_H
#define SYNTAX_ANALYSIS_H
#include "get_token.h"

enum AST_NODE_TYPE {
    PROGRAM,
    EXT_DEF_SEQ,
    EXT_VAR_DEF,
    TYPE_SPECIFIER, //类型限定符
    VAR_SEQ,
    FUNC_DEF,
    NONE_FORMAL_PARAM_SEQ, // 无形参情况下的占位符。避免特殊情况判断及空指针错误                                                            55
    FORMAL_PARAM_SEQ,
    FORMAL_PARAM,
    COMPOUND_STATEMENT,
    LOCAL_VAR_DEF_SEQ,
    LOCAL_VAR_DEF,
    STATEMENT_SEQ,
    STATEMENT,
    CONDITIONAL_STATEMENT,
    EXPRESSION,
    WORD, 
    ACTUAL_PARAM,
    RETURN_STATEMENT,
    UNKNOWN // 未知类型（即没有确定）
};

typedef struct WORD_INFO{
    int kind;
    char* data;
}WORD_INFO;
typedef struct AST_NODE{ // 采用孩子兄弟法存储多叉树
    AST_NODE_TYPE type;
    WORD_INFO word;
    struct AST_NODE* next_sibling;
    struct AST_NODE* first_child;
}AST_NODE;



bool is_const(int kind);

bool is_operator(int kind);

char* get_op(TOKEN_KIND kind);

struct AST_NODE** get_child(AST_NODE* node, int x);
#endif