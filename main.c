#include<stdio.h>
extern FILE* yyin;
extern int yydebug;
int main(int argc, char ** argv)
{
    if(argc <= 1) return 1;
    FILE *fp = fopen(argv[1], "r");
    if(!fp)
    {
        perror(argv[1]);
        return 1;
    }
    yydebug = 0;
    yyrestart(fp);
    yyparse();
    return 0;
}