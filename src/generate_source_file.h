#ifndef GENERATE_SOURCE_FILE_H
#define GENERATE_SOURCE_FILE_H
#include "display_AST.h"
#include "get_token.h"
#include "syntax_analysis.h"
void generate(AST_NODE* cur_node, FILE* fp, int indent);
#endif