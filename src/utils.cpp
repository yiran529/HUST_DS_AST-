#include "utils.h"

/**
 * 判断一个种类编号是不是类型关键字
 * @param kind 将要进行判断的单词种类编号
 * @return 如果是，则返回true；否则返回false
 */
bool is_type_specifier(int kind) { //判断是不是类型关键字
    return kind == INT ||
           kind == SHORT ||
           kind == LONG ||
           kind == CHAR ||
           kind == FLOAT ||
           kind == DOUBLE;
}