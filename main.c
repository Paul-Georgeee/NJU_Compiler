#include<stdio.h>
extern FILE* yyin;
int wrong;
void yyrestart(FILE * fp);
void yyparse();
struct TreeNode;
void traverse();
void printTree(struct TreeNode *p, int level);
void freeTree(struct TreeNode *p);
void printHashTable();
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
    yyrestart(fp);
    yyparse();
    if(wrong == 0)
    { 
        printTree(root, 0);
        traverse();
        printHashTable();
    }
    freeTree(root);
    return 0;
}