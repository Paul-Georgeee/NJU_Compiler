#include <stdio.h>
#include "tree.h"
#include "semantic.h"
#include "IRGenerator.h"
#include "controlFlowGraph.h"
#include "mips.h"
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
    FILE *IRfp = stdout;
    if(argc > 2)
        IRfp = fopen(argv[2], "w");
    if(wrong == 0)
    {
        printIR(IRfp, irList.head->next, irList.head);
        // constructCFG(irList.head->next->next, irList.head);
        genMips(irList.head->next, irList.head, stdout);
        freeIRList();
    }
    
    freeTree(root);
    return 0;
}