#include<stdio.h>
extern FILE* yyin;
int yydebug;
void yyrestart(FILE * fp);
void yyparse();
struct TreeNode;

void print(struct TreeNode *p);
struct TreeNode *root;

int main(int argc, char ** argv)
{
    if(argc <= 1) return 1;
    FILE *fp = fopen(argv[1], "r");
    if(!fp)
    {
        perror(argv[1]);
        return 1;
    }
    yydebug = 1;
    yyrestart(fp);
    yyparse();
    print(root);
    return 0;
}