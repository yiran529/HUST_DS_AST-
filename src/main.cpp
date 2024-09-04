#include<stdio.h>
#include<string.h>
#include "display_AST.h"
#include "get_token.h"
#include "syntax_analysis.h"
#include "generate_source_file.h"
extern char token_text[300];
extern char error_message[300];
extern int row, col;
extern AST_NODE* root;
int main()
{
    printf("Enter 1 to get token analysis.\n");
    printf("Enter 2 to get syntax analysis.\n");
    printf("Enter 3 to generate standard source file.\n");
    printf("Enter 0 to exit.\n");
    printf("Please Enter the path of the target file:\n");
    char path[1000];
    //scanf("%s", path);
    FILE *fp = fopen("E:\\DS_hw\\AST\\test\\syntax_analysis\\test11.txt", "r");
    while(1){
        printf("Which operation you want to perform: \n");
        char c; 
        rewind(fp); // 让文件指针回到开头  
        //scanf("%c", &c);getchar();
        c = '3';
        switch(c){
            case '1': 
                printf("Here is the result of token analysis: \n");
                int get_token_res;
                while((get_token_res = get_token(&fp)) != EOF) {
                    printf("%s %s\n", get_token_kind(get_token_res), token_text);
                }
                break;
            case '2':
                printf("Here is the result of syntax analysis: \n");
                if(build_program(&fp)) 
                    display_AST(root, 0);
                else {
                    printf("Error: %s\n", error_message);
                    printf("At row %d col %d", row, col - strlen(token_text) + 1);
                } 
                break;
            case '3':
                printf("Enter the path to which you want to put the standard source file: \n");
                char src_path[300];
                //scanf("%s", src_path);
                FILE *src_fp = fopen("E:\\DS_hw\\AST\\test\\syntax_analysis\\test12.txt", "w+");
                //fflush(src_fp);
                //fprintf(src_fp, "     \n");
                fflush(src_fp);
                if(build_program(&fp)) 
                    generate(root, src_fp, 0), fflush(src_fp);
                else {
                    printf("Error: %s\n", error_message);
                    printf("At row %d   col %d", row, col - strlen(token_text) + 1);
                } 
                fclose(src_fp);
                break;    
        }
        if(c == '0') break;
        break;
    }
    fclose(fp);
    return 0;
}