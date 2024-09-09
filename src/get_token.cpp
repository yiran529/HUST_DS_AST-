#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include "get_token.h"
#include "syntax_analysis.h"

extern int cur_kind; // 引用自syntax_analysis.cpp, 用以处理负号与负数的问题

int row = 1, col; // 记录文件指针当前在文件的第几行第几列

char token_text[300];

/**
 * 实现fgetc的功能的同时记录当前是第几列
 * @param fp 文件指针
 * @return 读取到的字符
 */
char my_fgetc(FILE* fp) {
    col++;
    return fgetc(fp);
}

/**
 * 实现my_ungetc的功能的同时记录当前是第几列
 * @param c 需要回到文件中的字符
 * @param fp 文件指针
 * @return 调用ungetc返回的结果
 */
int my_ungetc(char c, FILE* fp) {
    col--;
    return ungetc(c, fp);
}

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
 * 判断一个字符如果在其前面加上反斜杠是不是一个合法的转义字符（不包括数字表示形式）
 * @param c 需要判断的字符
 * @return true, 如果是合法的；否则false
 */
bool is_escape_character(char c) {
    return c == 'a' || c == 'b' || c == 'f' || c == 'n'
    || c == 'r' || c == 't' || c == 't' || c == 'v'
    || c == '\''|| c == '\\'|| c == '?' || c == '0';
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
    if(!strcmp(token_text, "string"))   return STRING;
    if(!strcmp(token_text, "unsigned")) return UNSIGNED;
    if(!strcmp(token_text, "signed"))   return SIGNED;
    if(!strcmp(token_text, "const"))    return CONST;
    if(!strcmp(token_text, "extern"))   return EXTERN;
    if(!strcmp(token_text, "auto"))     return AUTO;
    if(!strcmp(token_text, "static"))   return STATIC;
    if(!strcmp(token_text, "typedef"))  return TYPEDEF;
    if(!strcmp(token_text, "switch"))   return SWITCH;
    if(!strcmp(token_text, "case"))     return CASE;
    if(!strcmp(token_text, "default"))  return DEFAULT;
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
    char c = my_fgetc(fp);
    do {
        token_text[index++] = c;
        c = my_fgetc(fp);
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
    my_ungetc(c, fp);
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
        c = my_fgetc(fp);
        if(c == 'x' || c == 'X') {
            do {
                token_text[index++] = c;
                c = my_fgetc(fp);
            } while(is_hexadecimal_num(c));
            if(c == 'l' || c == 'L') {// 处理整型常量后面有l或L后缀的情况
                token_text[index++] = 'c';
                token_text[index] = '\0';
                return LONG_CONST;
            }
            my_ungetc(c, fp);
            token_text[index] = '\0';
            return INT_CONST;
        } else if(is_octal_num(c)) {
            do {
                token_text[index++] = c;
                c = my_fgetc(fp);
            } while(is_octal_num(c));
            if(c == 'l' || c == 'L') {// 处理整型常量后面有l或L后缀的情况
                token_text[index++] = c;
                token_text[index] = '\0';
                return LONG_CONST;
            }
            my_ungetc(c, fp);
            token_text[index] = '\0';
            return INT_CONST;
        } else if(is_num(c)){ // 八进制数表示错误
            my_ungetc(c, fp);
            *fp_pointer = fp;
            return ERROR_TOKEN;
        } else if(c == '.') { // 小数
            my_ungetc(c, fp);
            index--;
            c = '0';
        } else { // 单个数字0的情况需要特殊判断
            my_ungetc(c, fp);
            *fp_pointer = fp;
            token_text[1] = '\0';
            return INT_CONST;
        }
    }

    do {
        token_text[index++] = c;
        c = my_fgetc(fp);
    } while(is_num(c));

    if(c == 'l' || c == 'L') {// 处理整型常量后面有l或L后缀的情况
        token_text[index++] = c;
        token_text[index] = '\0';
        return LONG_CONST;
    }

    if(c == '.') { // 处理浮点型常数
        if(is_num(c = my_fgetc(fp))) {
            my_ungetc(c, fp);
            token_text[index++] = '.';
            TOKEN_KIND float_kind = process_float(token_text, index, &fp);
            *fp_pointer = fp;
            return float_kind; 
        } else return ERROR_TOKEN; // 需要考虑3.f类似的情况
    }
    my_ungetc(c, fp);
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
        case '=': c = my_fgetc(fp);
                  if(c == '=') {
                    *fp_pointer = fp; 
                    token_text[1] = '=';
                    token_text[2] = '\0'; 
                    return EQ;
                  }
                  my_ungetc(c, fp);
                  //*fp_pointer = fp; 为什么不能有这句？
                  return ASSIGN; 
        case '>': c = my_fgetc(fp);
                  if(c == '=') {
                    *fp_pointer = fp;
                    token_text[1] = '=';
                    token_text[2] = '\0';
                    return GREATEREQ;
                  } 
                  my_ungetc(c, fp);
                  *fp_pointer = fp;
                  return GREATER;
        case '<': c = my_fgetc(fp);
                  if(c == '=') {
                    *fp_pointer = fp;
                    token_text[1] = '=';
                    token_text[2] = '\0';
                    return LESSEQ;
                  } else if(is_letter(c) || is_num(c)) { //"可能"是包含的文件。由于文件命名规则复杂，现将其简化
                    int index = 1;
                    while(is_letter(c) || is_num(c) || c == '.'){
                        token_text[index++] = c;
                        c = my_fgetc(fp);
                    }
                    token_text[index++] = c;
                    token_text[index] = '\0';
                    if(c == '>') return FILE_NAME;
                    else {
                        while(index) my_ungetc(token_text[--index], fp);
                        c = my_fgetc(fp);
                        token_text[0] = c;
                        token_text[1] = '\0';
                        return LESS;
                    }
                  }
                  my_ungetc(c, fp); 
                  *fp_pointer = fp;
                  return LESS;
        case ';': return SEMI;
        case ',': return COMMA;
        case '&': c = my_fgetc(fp);
                  if(c == '&') {
                    *fp_pointer = fp;  
                    token_text[1] = '&';
                    token_text[2] = '\0';
                    return AND;
                  }
                  // my_ungetc(c, fp); 既然出现错误，可能就没必要再管文件指针了
                  return ERROR_TOKEN; //在本项目中，'&'不可能单独出现
        case '|': c = my_fgetc(fp);
                  if(c == '|') {
                    *fp_pointer = fp;  
                    token_text[1] = '|';
                    token_text[2] = '\0';
                    return OR;
                  }
                  // my_ungetc(c, fp); 既然出现错误，可能就没必要再管文件指针了
                  return ERROR_TOKEN; //在本项目中，'|'不可能单独出现
        case '+': return PLUS;
        case '-': {
                    char c = my_fgetc(fp);
                    if(cur_kind == SEMI || cur_kind == LP || cur_kind == LC || cur_kind == EQ
                    || cur_kind == NOTEQ || cur_kind == GREATER || cur_kind == LESS || cur_kind == GREATEREQ
                    || cur_kind == LESSEQ || cur_kind == ASSIGN) // 这里的cur_kind是上一个识别完成的单词
                        if(is_num(c)) {
                            TOKEN_KIND kind = process_number(token_text, 1, c, &fp);
                            *fp_pointer = fp;
                            return kind;
                        } else return ERROR_TOKEN;
                    my_ungetc(c, fp);
                    return MINUS;
                  }
        case '*': return MULTIPLY; 
        case '/': c = my_fgetc(fp);
                  if(c == '/') {
                    *fp_pointer = fp;  
                    token_text[1] = '/';
                    int index = 2;
                    while((c = my_fgetc(fp)) != '\n' && c != EOF) // 注意文件结束也可能让行注释结束
                        token_text[index++] = c;
                    row++;
                    token_text[index] = '\0';
                    *fp_pointer = fp;
                    return LINE_COMMENT; // 单行注释
                  } else if(c == '*') {
                    *fp_pointer = fp;  
                    token_text[1] = '*';
                    int index = 2;
                    while(1) {
                        c = my_fgetc(fp);
                        token_text[index++] = c;
                        if(c == '\n') row++;
                        if(c =='*') {
                            c = my_fgetc(fp);
                            if(c == '/') {
                                token_text[index++] = '/';
                                token_text[index] = '\0';
                                *fp_pointer = fp;
                                return BLOCK_COMMENT; // 块注释
                            } else my_ungetc(c, fp);
                        }
                        else if(c == EOF) {
                            token_text[index] = '\0';
                            return ERROR_TOKEN;
                        }
                    } 
                    return ERROR_TOKEN;
                  }
                  my_ungetc(c, fp);
                  *fp_pointer = fp;
                  return DEVIDE; 
        case '%': return MOD;
        case '(': return LP;
        case ')': return RP;
        case '[': return LB;
        case ']': return RB;
        case '{': return LC;
        case '}': return RC;
        case '!': c = my_fgetc(fp);
                  if(c == '=') {
                    *fp_pointer = fp;
                    token_text[1] = '=';
                    token_text[2] = '\0';
                    return NOTEQ;
                  }
                  my_ungetc(c, fp);
                  //*fp_pointer = fp;
                  return ERROR_TOKEN; // 暂时不考虑逻辑非
        case '\'':{
                  int index = 0;
                  token_text[index++] = c;
                  c = my_fgetc(fp);
                  if(is_letter(c) || is_num(c)) token_text[index++] = c;
                  else if(c == '\\') {
                    token_text[index++] = c;
                    c = my_fgetc(fp);
                    if(is_escape_character(c) && !is_octal_num(c = my_fgetc(fp))) { // 是转义字符（不包括数字表示形式）
                        my_ungetc(c, fp);
                        token_text[index++] = c;
                    } else if (is_octal_num(c)) { // 八进制表示的转义字符
                        do {
                            token_text[index++] = c;
                            c = my_fgetc(fp);
                        } while(is_octal_num(c) && index <= 5);
                        my_ungetc(c, fp);
                        if(index > 5) {
                            my_ungetc(c, fp);
                            return ERROR_TOKEN;
                        }
                    } else if (c == 'x') { // 十六进制表示的转义字符
                        token_text[index++] = c;
                        while(index <= 5 && is_hexadecimal_num(c = my_fgetc(fp)))
                            token_text[index++] = c;
                        my_ungetc(c, fp);
                        if(index > 5) {
                            my_ungetc(c, fp);
                            return ERROR_TOKEN;
                        }
                    } else {
                        my_ungetc(c, fp);
                        return ERROR_TOKEN;
                    }
                  } else {
                    my_ungetc(c, fp);
                    return ERROR_TOKEN;
                  }
                  c = my_fgetc(fp);
                  if(c != '\'') {
                    my_ungetc(c, fp);
                    return ERROR_TOKEN;
                  }
                  token_text[index++] = c;
                  token_text[index] = '\0';
                  *fp_pointer = fp;
                  return CHAR_CONST;
                }
        case '.': {
                    c = my_fgetc(fp); // 处理类似.124的情况
                    if(!is_num(c)) return ERROR_TOKEN;
                    my_ungetc(c, fp);
                    my_ungetc('.', fp);
                    c = '0';
                    TOKEN_KIND kind = process_number(token_text, 0, c, &fp);
                    *fp_pointer = fp;
                    return kind;
                  }
        case '#': {
                    c = my_fgetc(fp);
                    if(c == 'd' && (c = my_fgetc(fp)) == 'e' && (c = my_fgetc(fp)) == 'f' &&
                    (c = my_fgetc(fp)) == 'i' && (c = my_fgetc(fp)) == 'n' && (c = my_fgetc(fp)) == 'e' &&
                    (c = my_fgetc(fp)) == ' ') { // #define之后必须是空格
                        my_ungetc(c, fp);
                        strcpy(token_text, "#define");
                        return MACRO_DEFINE;
                    } else if(c == 'i' && (c = my_fgetc(fp)) == 'n' && (c = my_fgetc(fp)) == 'c' &&
                    (c = my_fgetc(fp)) == 'l' && (c = my_fgetc(fp)) == 'u' && (c = my_fgetc(fp)) == 'd' &&
                    (c = my_fgetc(fp)) == 'e') {
                        strcpy(token_text, "#include");
                        return FILE_INCLUDE;
                    }
                    return ERROR_TOKEN;
                  }
        case '\"':{
                    int index = 0;
                    token_text[index++] = c;
                    while((c = my_fgetc(fp)) != '\"' && c != '\n') token_text[index++] = c;
                    if(c=='\n') return ERROR_TOKEN;
                    token_text[index++] = c;
                    token_text[index] = '\0';
                    return STRING_CONST;
                  }
        case ':': token_text[0] = c;
                  token_text[1] = '\0';
                  return COLON;
        default:  if(feof(fp)) return EOF;
                  return ERROR_TOKEN;
    }
}

/**
 * 读取*fp_pointer所指向的源文件的下一个单词(包括注释)
 * @param fp_pointer 指向源文件当前读取位置的双重指针
 * @return 识别出的单词的种类码
 */
int get_all_token(FILE** fp_pointer) {
    
    int index = 0;
    FILE* fp = *fp_pointer;
    char c;
    while((c = my_fgetc(fp)) == ' ' || c == '\t' || c == '\n') {//过滤掉空白符号和换行
        if(c == '\n') row++, col = 0;
    } 

    if(is_letter(c) || c == '_') {
        do {
            token_text[index++] = c;
            c = my_fgetc(fp);
        } while(is_num(c) || is_letter(c) || c == '_');
        my_ungetc(c, fp);
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

char* get_token_kind(int kind) {
    switch(kind) {
        case ERROR_TOKEN: return "ERROR_TOKEN"; 
        case IDENT:       return "IDENT";
        case INT_CONST:   return "INT_CONST"; 
        case FLOAT_CONST: return "FLOAT_CONST"; 
        case DOUBLE_CONST:return "DOUBLE_CONST"; 
        case CHAR_CONST:  return "CHAR_CONST"; 
        case LONG_CONST:  return "LONG_CONST"; 
        case STRING_CONST:return "STRING_CONST";
        case INCLUDE:     return "INCLUDE"; 
        case STRUCT:      return "STRUCT"; 
        case TYPEDEF:     return "TYPEDEF";
        case INT:         return "INT"; 
        case SHORT:       return "SHORT";
        case FLOAT:       return "FLOAT";
        case DOUBLE:      return "DOUBLE";
        case CHAR:        return "CHAR";
        case LONG:        return "LONG";
        case STRING:      return "STRING";
        case VOID:        return "VOID";
        case IF:          return "IF";
        case ELSE:        return "ELSE";
        case EQ:          return "EQ";
        case GREATER:     return "GREATER";
        case LESS:        return "LESS";
        case GREATEREQ:   return "GREATEREQ";
        case LESSEQ:      return "LESSEQ";
        case NOTEQ:       return "NOTEQ";
        case ASSIGN:      return "ASSIGN";
        case SEMI:        return "SEMI";
        case COMMA:       return "COMMA"; 
        case BREAK:       return "BREAK";
        case CONTINUE:    return "CONTINUE"; 
        case RETURN:      return "RETURN";
        case FOR:         return "FOR";
        case WHILE:       return "WHILE";
        case DO:          return "DO";
        case AND:         return "AND"; 
        case OR:          return "OR";
        case PLUS:        return "PLUS";
        case MINUS:       return "MINUS";
        case MULTIPLY:    return "MULTIPLY"; 
        case DEVIDE:      return "DEVIDE";
        case MOD:         return "MOD";
        case LP:          return "LP";
        case RP:          return "RP";
        case LB:          return "LB";
        case RB:          return "RB";
        case LC:          return "LC";
        case RC:          return "RC";
        case MACRO_DEFINE:return "MACRO_DEFINE";
        case FILE_INCLUDE:return "FILE_INCLUDE";
        case FILE_NAME:   return "FIME_NAME";
        case LINE_COMMENT:return "LINE_COMMENT";
        case BLOCK_COMMENT:return "BLOCK_COMMENT";
        case UNSIGNED:    return "UNSIGNED";
        case SIGNED:      return "SIGNED";
        case EXTERN:      return "EXTERN";
        case STATIC:      return "STATIC";
        case AUTO:        return "AUTO";
        case CONST:       return "CONST";
        case SWITCH:      return "SWITCH";
        case CASE:        return "CASE";
        case COLON:       return "COLON";
        case DEFAULT:     return "DEFAULT";
        default:          return "ERROR!";
    }
}


