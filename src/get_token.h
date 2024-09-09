#ifndef GET_TOKEN_H
#define GET_TOKEN_H

#include<stdio.h>
#include<stdlib.h>

bool is_num(char c);

enum TOKEN_KIND{
    ERROR_TOKEN, 
    IDENT, 
    INT_CONST, 
    FLOAT_CONST, 
    DOUBLE_CONST, 
    CHAR_CONST, 
    LONG_CONST, 
    STRING_CONST,
    INCLUDE, 
    STRUCT, 
    TYPEDEF,
    INT, 
    SHORT,
    FLOAT, 
    DOUBLE, 
    CHAR, 
    LONG, 
    STRING,
    VOID,
    IF, 
    ELSE, 
    EQ, 
    GREATER, 
    LESS, 
    GREATEREQ,// >=
    LESSEQ,// <=
    NOTEQ,
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
    HASH,
    MACRO_DEFINE, // #define
    FILE_INCLUDE, // #include
    FILE_NAME, // <....>
    LINE_COMMENT, // 单行注释
    BLOCK_COMMENT, // 多行注释
    COMMA_OR_RP, // 要么是逗号要么是右括号，用以处理函数调用中形参的问题
    UNSIGNED,
    SIGNED,
    CONST,
    EXTERN,
    STATIC,
    AUTO,
    SWITCH,
    CASE,
    DEFAULT,
    COLON // 冒号
};
//还差两个注释符号

/**
 * 读取*fp_pointer所指向的源文件的下一个单词(不包括注释)
 * @param fp_pointer 指向源文件当前读取位置的双重指针
 * @return 识别出的单词的种类码
 */
int get_token(FILE** fp_pointer);

/**
 * 读取*fp_pointer所指向的源文件的下一个单词(包括注释)
 * @param fp_pointer 指向源文件当前读取位置的双重指针
 * @return 识别出的单词的种类码
 */
int get_all_token(FILE** fp_pointer);
/**
 * 根据给定的单词编号返回对应的单词种类的字符串表示
 * @param kind 单词编号
 * @return 对应的单词种类字符串表示
 */
char* get_token_kind(int kind);

char my_fgetc(FILE* fp);

int my_ungetc(char c, FILE* fp);

bool is_letter(char c);
#endif