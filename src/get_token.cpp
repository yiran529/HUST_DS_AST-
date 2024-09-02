#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include "get_token.h"
#include "syntax_analysis.h"

extern int cur_kind; // 引用自syntax_analysis.cpp, 用以处理负号与负数的问题

int lines_num;

char token_text[300];

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
    if(!strcmp(token_text, "int"))      return INT;
    if(!strcmp(token_text, "short"))    return SHORT;
    if(!strcmp(token_text, "char"))     return CHAR;
    if(!strcmp(token_text, "float"))    return FLOAT;
    if(!strcmp(token_text, "double"))   return DOUBLE;
    if(!strcmp(token_text, "long"))     return LONG;
    if(!strcmp(token_text, "void"))     return VOID;
    if(!strcmp(token_text, "if"))       return IF;
    if(!strcmp(token_text, "else"))     return ELSE;
    if(!strcmp(token_text, "break"))    return BREAK;
    if(!strcmp(token_text, "continue")) return CONTINUE;
    if(!strcmp(token_text, "return"))   return RETURN;
    if(!strcmp(token_text, "for"))      return FOR;
    if(!strcmp(token_text, "while"))    return WHILE;
    if(!strcmp(token_text, "do"))       return DO;
    if(!strcmp(token_text, "include"))  return INCLUDE;
    if(!strcmp(token_text, "struct"))   return STRUCT;
    if(!strcmp(token_text, "typedef"))  return TYPEDEF;
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
    // if(c == 'l' || c == 'L') {
    //     token_text[index++] = c;
    //     token_text[index] = '\0';
    //     return DOUBLE_CONST;
    // }
    if(c == 'f' || c == 'F') {
        token_text[index++] = c;
        token_text[index] = '\0';
        return FLOAT_CONST;
    }
    token_text[index] = '\0';
    ungetc(c, fp);
    *fp_pointer = fp;
    return DOUBLE_CONST;
}

/**
 * 判断一个字符表示的是否是十六进制的数（0~9， a~f，A~F）
 * @param c 将要进行判断的字符
 * @return 如果是十六进制的数，则返回true；否则返回false
 */
bool is_hexadecimal_num(char c) {
    return c >= '0' && c <='9' || c >='a' && c <= 'f' || c >= 'A' && c <= 'F';
}

/**
 * 判断一个字符表示的是否是八进制的数（0~7）
 * @param c 将要进行判断的字符
 * @return 如果是八进制的数，则返回true；否则返回false
 */
bool is_octal_num(char c) {
    return c >= '0' && c <='7';
}


/**
 * 读取数字（不含符号）
 * @param token_text 存储数字字符串的数组(要求如果原来的数有负号，则需要它已经存储在token_text中)
 * @param index 从token_text的第几位开始存储数字（从0开始计）
 * @param c 数字字符串的第一个表示数字的字符(该字符未被放入token_text中)
 * @param fp_pointer 输入文件当前的文件指针的地址
 * @return 数字常量对应的种类编码
 */
TOKEN_KIND process_number(char* token_text, int index, char c, FILE** fp_pointer) {
    FILE* fp = *fp_pointer;

    if(c == '0') {
        token_text[index++] = c;
        c = fgetc(fp);
        if(c == 'x') {
            do {
                token_text[index++] = c;
                c = fgetc(fp);
            } while(is_hexadecimal_num(c));
            if(c == 'l' || c == 'L') {// 处理整型常量后面有l或L后缀的情况
                token_text[index++] = 'c';
                token_text[index] = '\0';
                return LONG_CONST;
            }
            ungetc(c, fp);
            token_text[index] = '\0';
            return INT_CONST;
        } else if(is_octal_num(c)) {
            do {
                token_text[index++] = c;
                c = fgetc(fp);
            } while(is_octal_num(c));
            if(c == 'l' || c == 'L') {// 处理整型常量后面有l或L后缀的情况
                token_text[index++] = c;
                token_text[index] = '\0';
                return LONG_CONST;
            }
            ungetc(c, fp);
            token_text[index] = '\0';
            return INT_CONST;
        } else if(is_num(c)){ // 八进制数表示错误
            ungetc(c, fp);
            *fp_pointer = fp;
            return ERROR_TOKEN;
        } else { // 单个数字0的情况需要特殊判断
            ungetc(c, fp);
            *fp_pointer = fp;
            token_text[1] = '\0';
            return INT_CONST;
        }
    }

    do {
        token_text[index++] = c;
        c = fgetc(fp);
    } while(is_num(c));

    if(c == 'l' || c == 'L') {// 处理整型常量后面有l或L后缀的情况
        token_text[index++] = c;
        token_text[index] = '\0';
        return LONG_CONST;
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
    ungetc(c, fp);
    token_text[index] = '\0';
    *fp_pointer = fp;
    return INT_CONST;
}


/**
 * 处理非错误符号，变量，数值常量的其它情况
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
                  //*fp_pointer = fp; 为什么不能有这句？
                  return ASSIGN; 
        case '>': c = fgetc(fp);
                  if(c == '=') {
                    *fp_pointer = fp;
                    token_text[1] = '=';
                    token_text[2] = '\0';
                    return GREATEREQ;
                  }
                  ungetc(c, fp);
                  *fp_pointer = fp;
                  return GREATER;
        case '<': c = fgetc(fp);
                  if(c == '=') {
                    *fp_pointer = fp;
                    token_text[1] = '=';
                    token_text[2] = '\0';
                    return LESSEQ;
                  }
                  ungetc(c, fp); 
                  *fp_pointer = fp;
                  return LESS;
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
        case '+': return PLUS;
        case '-': {
                    char c = fgetc(fp);
                    if(cur_kind == SEMI || cur_kind == LP || cur_kind == LC) // 这里的cur_kind是上一个识别完成的单词
                        if(is_num(c)) {
                            TOKEN_KIND kind = process_number(token_text, 1, c, &fp);
                            *fp_pointer = fp;
                            return kind;
                        } else return ERROR_TOKEN;
                    ungetc(c, fp);
                    return MINUS;
                  }
        case '*': c = fgetc(fp);
                  if(c == '/') {
                    *fp_pointer = fp;  
                    token_text[1] = '/';
                    token_text[2] = '\0';
                    return END_OF_MULTILINE_COMMENT;
                  } 
                  ungetc(c, fp);
                  *fp_pointer = fp;
                  return MULTIPLY; 
        case '/': c = fgetc(fp);
                  if(c == '/') {
                    *fp_pointer = fp;  
                    token_text[1] = '/';
                    token_text[2] = '\0';
                    return DOUBLE_SLASH;
                  } else if(c == '*') {
                    *fp_pointer = fp;  
                    token_text[1] = '*';
                    token_text[2] = '\0';
                    return START_OF_MULTILINE_COMMENT;
                  }
                  ungetc(c, fp);
                  *fp_pointer = fp;
                  return DEVIDE; 
        case '%': return MOD;
        case '(': return LP;
        case ')': return RP;
        case '[': return LB;
        case ']': return RB;
        case '{': return LC;
        case '}': return RC;
        case '#': return HASH;
        case '!': c = fgetc(fp);
                  if(c == '=') {
                    *fp_pointer = fp;
                    token_text[1] = '=';
                    token_text[2] = '\0';
                    return NOTEQ;
                  }
                  ungetc(c, fp);
                  //*fp_pointer = fp;
                  return ERROR_TOKEN; // 暂时不考虑逻辑非
        case '\'':c = fgetc(fp);
                  if(!is_letter(c)) {
                      ungetc(c, fp);
                      return ERROR_TOKEN;
                  }
                  token_text[1] = c;
                  c = fgetc(fp);
                  if(c != '\'') {
                      ungetc(c, fp);
                      return ERROR_TOKEN;
                  }
                  token_text[2] = c;
                  token_text[3] = '\0';
                  *fp_pointer = fp;
                  return CHAR_CONST;
        default:  if(feof(fp)) return EOF;
                  return ERROR_TOKEN;
    }
}


int get_token(FILE** fp_pointer) {
    
    int index = 0;
    FILE* fp = *fp_pointer;
    char c;
    while((c = fgetc(fp)) == ' ' || c == '\t' || c == '\n') {if(c == '\n') lines_num++;} //过滤掉空白符号和换行

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
        TOKEN_KIND kind = process_number(token_text, 0, c, &fp);
        *fp_pointer = fp;
        return kind;
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