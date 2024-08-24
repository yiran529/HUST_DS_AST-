#include "get_token.h"
enum AST_NODE_TYPE {
    PROGRAM,
    EXT_DEF_SEQ,
    EXT_DEF,
    EXT_VAR_DEF,
    TYPE_SPECIFIER, //类型限定符
    VAR_SEQ,
    FUNC_DEF,
    FORMAL_PARAM_SEQ,
    FORMAL_PARAM,
    COMPOUND_STATEMENT,
    LOCAL_VAR_DEF_SEQ,
    LOCAL_VAR_DEF,
    STATEMENT_SEQ,
    STATEMENT,
    EXPRESSION,
    WORD, 
    ACTUAL_PARAM
};

typedef struct { // 采用孩子兄弟法存储多叉树
    AST_NODE_TYPE type;
    struct {
        TOKEN_KIND kind;
        char* data;
    }word;
    AST_NODE* next_sibling;
    AST_NODE* first_child;
}AST_NODE;

AST_NODE* root;

AST_NODE** AST;

int cur_kind; // 当前读到的单词的种类编号

