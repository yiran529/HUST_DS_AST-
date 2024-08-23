#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include "get_token.h"

/**
 * 判断一个字符是不是表示数字
 * @param c 一个字符
 * @return true，如果是数字；否则false
 */
bool is_num(char c) {
    return c >= '0' && c <= '9';
}

/**
 * 判断一个字符是不是表示字母
 * @param c 一个字符
 * @return true，如果是字母；否则false
 */
bool is_letter(char c) {
    return c >= 'A' && c <= 'Z' || c >= 'a' && c <='z';
}

/**
 * 判断一个字符串是关键字还是标识符；若是关键字，是哪个关键字
 * @param token_text 待判断的字符串（要求必须是标识符或关键字）
 * @return 该字符串对应的种类码
 */
TOKEN_KIND get_keyword(char* token_text) {
    if(!strcmp(token_text, "int")) return INT;
    if(!strcmp(token_text, "char"))return CHAR;
    if(!strcmp(token_text, "float")) return FLOAT;
    if(!strcmp(token_text, "double")) return DOUBLE;
    if(!strcmp(token_text, "bool")) return BOOL;
    if(!strcmp(token_text, "long")) return LONG;
    if(!strcmp(token_text, "if"))return IF;
    if(!strcmp(token_text, "else")) return ELSE;
    if(!strcmp(token_text, "break"))return BREAK;
    if(!strcmp(token_text, "continue")) return CONTINUE;
    if(!strcmp(token_text, "return"))return RETURN;
    if(!strcmp(token_text, "for")) return FOR;
    if(!strcmp(token_text, "while"))return WHILE;
    if(!strcmp(token_text, "do")) return DO;
    if(!strcmp(token_text, "include")) return INCLUDE;
    if(!strcmp(token_text, "struct")) return STRUCT;
    if(!strcmp(token_text, "typedef")) return TYPEDEF;
    return IDENT; 
    //TODO;//自身值怎么处理
}

/**
 * 读取浮点型常量的小数部分
 * @param token_text 存储浮点型常量字符串的数组(要求已经填写过整数部分及小数点)
 * @param index 从token_text的第几位开始存储小数部分（从0开始计）
 * @param fp_pointer 输入文件当前的文件指针的地址
 * @return 如果有L或l后缀，则为DOUBLE_CONST; 否则是 FLOAT_CONST
 */
TOKEN_KIND process_float(char* token_text, int index, FILE** fp_pointer) {
    FILE* fp = *fp_pointer;
    char c = fgetc(fp);
    do {
        token_text[index++] = c;
        c = fgetc(fp);
    } while(is_num(c));
    token_text[index] = '\0';
    if(c == 'l' || c == 'L') return DOUBLE_CONST;
    if(c == 'f' || c == 'F') return FLOAT_CONST;
    ungetc(c, fp);
    *fp_pointer = fp;
    return FLOAT_CONST;
}

/**
 * 读取数字（不含符号）
 * @param token_text 存储数字字符串的数组(要求如果原来的数有负号，则需要它已经存储在token_text中)
 * @param index 从token_text的第几位开始存储数字（从0开始计）
 * @param fp_pointer 输入文件当前的文件指针的地址
 * @return 数字常量对应的种类编码
 */
TOKEN_KIND process_number(char* token_text, int index, FILE** fp_pointer) {

}
/**
 * 处理非错误符号，变量的其它情况
 * @param c 读取到的第一个字符
 * @param token_text 存储词的字符型数组
 * @param fp_pointer 输入文件当前的文件指针的地址
 * @return 当前读取到的单词的种类编号；如果到达文件末尾，则返回EOF
 */
int process_others(char c, char* token_text, FILE** fp_pointer) {
    FILE* fp = *fp_pointer;
    switch(c) {
        case '=': c = fgetc(fp);
                  if(c == '=') {
                    *fp_pointer = fp; 
                    token_text[1] = '=';
                    token_text[2] = '\0'; 
                    return EQ;
                  }
                  ungetc(c, fp);
                  return ASSIGN;
        case '>': return GREATER;
        case '<': return LESS;
        case ';': return SEMI;
        case ',': return COMMA;
        case '&': c = fgetc(fp);
                  if(c == '&') {
                    *fp_pointer = fp;  
                    token_text[1] = '&';
                    token_text[2] = '\0';
                    return AND;
                  }
                  // ungetc(c, fp); 既然出现错误，可能就没必要再管文件指针了
                  return ERROR_TOKEN; //在本项目中，'&'不可能单独出现
        case '|': c = fgetc(fp);
                  if(c == '|') {
                    *fp_pointer = fp;  
                    token_text[1] = '|';
                    token_text[2] = '\0';
                    return OR;
                  }
                  // ungetc(c, fp); 既然出现错误，可能就没必要再管文件指针了
                  return ERROR_TOKEN; //在本项目中，'|'不可能单独出现
        case '+': return PLUS; // 需不需要处理"++"时要报错的情况？还是丢给语法分析模块处理？
        case '-': c = fgetc(fp);
                  if(!is_num(c)) {
                    ungetc(c, fp);
                    return MINUS;
                  }

        case '*': return MULTIPLY;
        case '/': return DEVIDE;
        case '%': return MOD;
        case '(': return LP;
        case ')': return RP;
        case '[': return LB;
        case ']': return RB;
        case '{': return LC;
        case '}': return RC;
        case '#': return HASH;
        default:  if(feof(fp)) return EOF;
                  return ERROR_TOKEN;
    }
}


int get_token(FILE** fp_pointer) {
    
    int index = 0;
    FILE* fp = *fp_pointer;
    char c;
    while((c = fgetc(fp)) == ' ' || c == '\n') {if(c == '\n') lines_num++;} //过滤掉空白符号和换行

    if(is_letter(c)) {
        do {
            token_text[index++] = c;
            c = fgetc(fp);
        } while(is_num(c) || is_letter(c));
        ungetc(c, fp);
        token_text[index] = '\0';
        *fp_pointer = fp;
        return get_keyword(token_text);
    }

    if(is_num(c)) {
        do {
            token_text[index++] = c;
            c = fgetc(fp);
        } while(is_num(c));

        if(c == 'l' || c == 'L') {// 处理整型常量后面有l或L后缀的情况
            token_text[index] = '\0';
            return LONG;
        }

        if(c == '.') { // 处理浮点型常数
            if(is_num(c = fgetc(fp))) {
                ungetc(c, fp);
                token_text[index++] = '.';
                TOKEN_KIND float_kind = process_float(token_text, index, &fp);
                *fp_pointer = fp;
                return float_kind; 
            } else return ERROR_TOKEN; // 需要考虑3.f类似的情况
        }
       //TODO;// 0x233 负数
        ungetc(c, fp);
        token_text[index] = '\0';
        *fp_pointer = fp;
        return INT_CONST;
        //TODO;//处理自身值
    }

    token_text[0] = c;
    token_text[1] = '\0';
    int kind = process_others(c, token_text, &fp);
    *fp_pointer = fp;
    return kind;
}

// int main() {
//     char path[1000];
//     scanf("%s", path);
//     FILE *fp = fopen(path, "r");
//     int get_token_res;
//     while((get_token_res = get_token(&fp)) != EOF) {
//         printf("%d %s\n", get_token_res, token_text);
//     }
//     printf("%d", lines_num);
//     return 0;
// }