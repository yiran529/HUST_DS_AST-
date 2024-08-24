#include<stdio.h>
#include <stdlib.h>
bool is_num(char c);
enum TOKEN_KIND{
    ERROR_TOKEN, 
    IDENT, 
    INT_CONST, 
    FLOAT_CONST, 
    DOUBLE_CONST, 
    CHAR_CONST, 
    LONG_CONST, 
    INCLUDE, 
    STRUCT, 
    TYPEDEF,
    INT, 
    SHORT,
    FLOAT, 
    DOUBLE, 
    CHAR, 
    LONG, 
    IF, 
    ELSE, 
    EQ, 
    GREATER, 
    LESS, 
    GREATEREQ,// >=
    LESSEQ,// <=
    ASSIGN, 
    SEMI, 
    COMMA, 
    BREAK, 
    CONTINUE, 
    RETURN, 
    FOR, 
    WHILE, 
    DO, 
    AND, 
    OR, 
    PLUS, 
    MINUS, 
    MULTIPLY, 
    DEVIDE, 
    MOD,
    LP, 
    RP,  
    LB, //'['
    RB, //']'
    LC, //'{'
    RC, //'}'
    HASH //'#'
};
//define?????  <>????????
int lines_num;

char token_text[300];

/**
 * 读取*fp_pointer所指向的源文件的下一个单词
 * @param fp_pointer 指向源文件当前读取位置的双重指针
 * @return 识别出的单词的种类码
 */
int get_token(FILE** fp_pointer);