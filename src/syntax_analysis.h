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
    EXPRESSION,
    WORD, 
    ACTUAL_PARAM,
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


int operator_cmp[200][200] = 
{
    {1, 1, 0, 0, 0, 1, 3, 1, 1, 1},
    {1, 1, 0, 0, 0, 1, 3, 1, 1, 1},
    {1, 1, 1, 1, 0, 1, 3, 1, 1, 1},
    {1, 1, 1, 1, 0, 1, 3, 1, 1, 1},
    {0, 0, 0, 0, 0, 1, 3, 1, 1, 1},
    {0, 0, 0, 0, 0, 2, 3, 1, 1, 1},
    {1, 1, 1, 1, 1, 3, 3, 1, 1, 1},
    {0, 0, 0, 0, 0, 3, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 1, 3, 1, 1, 1},
    {0, 0, 0, 0, 0, 1, 3, 0, 1, 1}
};
// 0 表示小于


#endif