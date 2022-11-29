#include <stdio.h>
#include "tree.h"
#include "semantic.h"
#include "IRGenerator.h"
#include "controlFlowGraph.h"
#include "mips.h"
#include <assert.h>
extern FILE* yyin;
int wrong;
void yyrestart(FILE * fp);
void yyparse();
void printTree(struct TreeNode *p, int level);
void freeTree(struct TreeNode *p);
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
        initIRGenerator();
        traverse();
    }
    FILE *mipsfp = stdout;
    if(argc > 2)
        mipsfp = fopen(argv[2], "w");
    if(wrong == 0)
    {
        genMips(irList.head->next, irList.head, mipsfp);
        freeIRList();
    }
    
    freeTree(root);
    return 0;
}