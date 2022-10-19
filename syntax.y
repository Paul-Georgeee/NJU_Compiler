%{
    #define YYERROR_VERBOSE
    void yyerror(const char *msg);
    #define YYSTYPE struct TreeNode*
    int printErrorFlag = 1;
    int wrong = 0;

    #include "tree.h"

    #include "lex.yy.c"

    void printError(int line, char *descprition)
    {
        wrong = 1;
        fprintf(stderr, "Error type B at Line %d: %s\n", line, descprition);
    }
    
    #define matchFakeRule(line, descprition, node)\
    {\
        printError(line, descprition);\
        node = NULL;\
        printErrorFlag = 0;\
        YYERROR;\
    }
    
    #define matchError(line, descprition, node)\
    {\
        if(printErrorFlag == 1)\
            printError(line, descprition);\
        else\
            printErrorFlag = 1;\
        node = NULL;\
    }

%}

%nonassoc MISSERROR
%token RETURN IF ELSE WHILE TYPE
%token LC RC
%token STRUCT COMMA 
%precedence SEMI ID INT FLOAT
%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT
%right NEG
%left LP RP LB RB DOT

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%
Program : ExtDefList {
        struct TreeNode * child[1] = {$1};
        $$ = matchRule("Program", child, 1, @1.first_line);
        root = $$;
    }
    ;
    
ExtDefList : ExtDef ExtDefList {
        struct TreeNode * child[2] = {$1, $2};
        $$ = matchRule("ExtDefList", child, 2, @1.first_line);
    }
    | %empty {$$ = NULL;}
    ;

ExtDef : Specifier ExtDecList SEMI {
        struct TreeNode * child[3] = {$1, $2, $3};
        $$ = matchRule("ExtDef", child, 3, @1.first_line);
    }

    | Specifier ExtDecList %prec MISSERROR {
        matchFakeRule(@1.first_line, "Miss \";\" in ExtDef", $$);
    }
    
    | Specifier SEMI {
        struct TreeNode * child[2] = {$1, $2};
        $$ = matchRule("ExtDef", child, 2, @1.first_line);
    }
    | Specifier %prec MISSERROR {
        matchFakeRule(@1.first_line, "Miss \";\" in ExtDef", $$);
    }
    
    | Specifier FunDec CompSt {
        struct TreeNode * child[3] = {$1, $2, $3};
        $$ = matchRule("ExtDef", child, 3, @1.first_line);
    }

    | Specifier FunDec SEMI {
        struct TreeNode * child[3] = {$1, $2, $3};
        $$ = matchRule("ExtDef", child, 3, @1.first_line);
    }

    | error SEMI {
        matchError(@1.first_line, "Error in ExtDef", $$);
    }
    ;

ExtDecList : VarDec {
        struct TreeNode * child[1] = {$1};
        $$ = matchRule("ExtDecList", child, 1, @1.first_line);
    }
    | VarDec COMMA ExtDecList
    {
        struct TreeNode * child[3] = {$1, $2, $3};
        $$ = matchRule("ExtDecList", child, 3, @1.first_line);
    }
    ;

Specifier : TYPE {
        struct TreeNode * child[1] = {$1};
        $$ = matchRule("Specifier", child, 1, @1.first_line);
    }
    | StructSpecifier {
        struct TreeNode * child[1] = {$1};
        $$ = matchRule("Specifier", child, 1, @1.first_line);
    }
    ;

StructSpecifier : STRUCT OptTag LC DefList RC {
        struct TreeNode * child[5] = {$1, $2, $3, $4, $5};
        $$ = matchRule("StructSpecifier", child, 5, @1.first_line);  
    }

    | STRUCT Tag {
        struct TreeNode * child[2] = {$1, $2};
        $$ = matchRule("StructSpecifier", child, 2, @1.first_line);
    }

    | error RC {
        matchError(@1.first_line, "Error in Struct def", $$);
    }
    ;

OptTag : ID {
        struct TreeNode * child[1] = {$1};
        $$ = matchRule("OptTag", child, 1, @1.first_line);
    }
    | %empty{
        $$ = NULL;
    }
    ;
Tag : ID {
        struct TreeNode * child[1] = {$1};
        $$ = matchRule("Tag", child, 1, @1.first_line);
    }
    ;

VarDec : ID {
        struct TreeNode * child[1] = {$1};
        $$ = matchRule("VarDec", child, 1, @1.first_line);
    }

    | VarDec LB INT RB {
        struct TreeNode * child[4] = {$1, $2, $3, $4};
        $$ = matchRule("VarDec", child, 4, @1.first_line);
    }

    | VarDec LB error RB {
        matchError(@1.first_line, "Error in VacDec", $$);
    }
    ;

FunDec : ID LP VarList RP {
        struct TreeNode * child[4] = {$1, $2, $3, $4};
        $$ = matchRule("FunDec", child, 4, @1.first_line);
    }

    | ID LP RP {
        struct TreeNode * child[3] = {$1, $2, $3};
        $$ = matchRule("FunDec", child, 3, @1.first_line);
    }

    | ID LP error RP {
        matchError(@3.first_line, "Error in Func Args", $$);
    }

    ;

VarList : ParamDec COMMA VarList {
        struct TreeNode * child[3] = {$1, $2, $3};
        $$ = matchRule("VarList", child, 3, @1.first_line);
    }

    | ParamDec {
        struct TreeNode * child[1] = {$1};
        $$ = matchRule("VarList", child, 1, @1.first_line);
    }
    ;

ParamDec : Specifier VarDec {
        struct TreeNode *child[2] = {$1, $2};
        $$ = matchRule("ParamDec", child, 2, @1.first_line);
    }
    ;

CompSt : LC DefList StmtList RC{
        struct TreeNode *child[4] = {$1, $2, $3, $4};
        $$ = matchRule("CompSt", child, 4, @1.first_line);
    }

    ;

StmtList : Stmt StmtList {
        struct TreeNode *child[2] = {$1, $2};
        $$ = matchRule("StmtList", child, 2, @1.first_line);
    }

    | %empty {$$ = NULL;}
    ;
Stmt : Exp SEMI {
        struct TreeNode *child[2] = {$1, $2};
        $$ = matchRule("Stmt", child, 2, @1.first_line);
    }

    | Exp %prec MISSERROR {
        matchFakeRule(@1.first_line, "Miss \";\" in Exp Stmt", $$);    
    }

    | CompSt {
        struct TreeNode *child[1] = {$1};
        $$ = matchRule("Stmt", child, 1, @1.first_line);
    }
    
    | RETURN Exp SEMI {
        struct TreeNode *child[3] = {$1, $2, $3};
        $$ = matchRule("Stmt", child, 3, @1.first_line);    
    }
    | RETURN %prec MISSERROR {
        matchFakeRule(@1.first_line, "Miss \";\" in Return Stmt", $$);    
    }
    
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {
        struct TreeNode *child[5] = {$1, $2, $3, $4, $5};
        $$ = matchRule("Stmt", child, 5, @1.first_line);   
    }
    | IF LP error RP Stmt %prec LOWER_THAN_ELSE {
        matchError(@3.first_line, "Error in if Exp", $$);
    }

    | IF LP Exp RP Stmt ELSE Stmt {
        struct TreeNode *child[7] = {$1, $2, $3, $4, $5, $6, $7};
        $$ = matchRule("Stmt", child, 7, @1.first_line);  
    }
    | IF LP error RP Stmt ELSE Stmt {
        matchError(@3.first_line, "Error in if else Exp", $$);
    }

    | ELSE error {
        matchError(@1.first_line, "unmatch if else", $$);
    }
    
    | WHILE LP Exp RP Stmt 
    {
        struct TreeNode *child[5] = {$1, $2, $3, $4, $5};
        $$ = matchRule("Stmt", child, 5, @1.first_line); 
    }
    | WHILE LP error RP Stmt {
        matchError(@3.first_line, "Error in while Exp", $$);
    }

    | error SEMI {
        matchError(@1.first_line, "Error in Stmt", $$);
    }
    ;

DefList : Def DefList {
        struct TreeNode *child[2] = {$1, $2};
        $$ = matchRule("DefList", child, 2, @1.first_line);
    }

    | %empty {$$ = NULL;}
    ;

Def : Specifier DecList SEMI {
        struct TreeNode *child[3] = {$1, $2, $3};
        $$ = matchRule("Def", child, 3, @1.first_line);
    }
    | Specifier DecList %prec MISSERROR {
        matchFakeRule(@2.first_line, "Miss \";\" in Def", $$);
    }
    | error SEMI {
        matchError(@1.first_line, "Error in Def", $$);
    }
    ;

DecList : Dec {
        struct TreeNode *child[1] = {$1};
        $$ = matchRule("DecList", child, 1, @1.first_line);
    }
    | Dec COMMA DecList {
        struct TreeNode *child[3] = {$1, $2, $3};
        $$ = matchRule("DecList", child, 3, @1.first_line);
    }
    ;

Dec : VarDec {
        struct TreeNode *child[1] = {$1};
        $$ = matchRule("Dec", child, 1, @1.first_line);
    }
    | VarDec ASSIGNOP Exp {
        struct TreeNode *child[3] = {$1, $2, $3};
        $$ = matchRule("Dec", child, 3, @1.first_line);
    }
    ;

Exp : Exp ASSIGNOP Exp {
        struct TreeNode *child[3] = {$1, $2, $3};
        $$ = matchRule("Exp", child, 3, @1.first_line);
    }
    | Exp AND Exp {
        struct TreeNode *child[3] = {$1, $2, $3};
        $$ = matchRule("Exp", child, 3, @1.first_line);
    }
    | Exp OR Exp {
        struct TreeNode *child[3] = {$1, $2, $3};
        $$ = matchRule("Exp", child, 3, @1.first_line);
    }
    | Exp RELOP Exp {
        struct TreeNode *child[3] = {$1, $2, $3};
        $$ = matchRule("Exp", child, 3, @1.first_line);
    }
    | Exp PLUS Exp {
        struct TreeNode *child[3] = {$1, $2, $3};
        $$ = matchRule("Exp", child, 3, @1.first_line);
    }
    | Exp MINUS Exp {
        struct TreeNode *child[3] = {$1, $2, $3};
        $$ = matchRule("Exp", child, 3, @1.first_line);
    }
    | Exp STAR Exp {
        struct TreeNode *child[3] = {$1, $2, $3};
        $$ = matchRule("Exp", child, 3, @1.first_line);
    }
    | Exp DIV Exp {
        struct TreeNode *child[3] = {$1, $2, $3};
        $$ = matchRule("Exp", child, 3, @1.first_line);
    }
    | LP Exp RP {
        struct TreeNode *child[3] = {$1, $2, $3};
        $$ = matchRule("Exp", child, 3, @1.first_line);
    }
    | MINUS Exp %prec NEG{
        struct TreeNode *child[2] = {$1, $2};
        $$ = matchRule("Exp", child, 2, @1.first_line);
    }
    | NOT Exp {
        struct TreeNode *child[2] = {$1, $2};
        $$ = matchRule("Exp", child, 2, @1.first_line);
    }
    | ID LP Args RP  {
        struct TreeNode *child[4] = {$1, $2, $3, $4};
        $$ = matchRule("Exp", child, 4, @1.first_line);
    }
    | ID LP RP {
        struct TreeNode *child[3] = {$1, $2, $3};
        $$ = matchRule("Exp", child, 3, @1.first_line);
    }
    | ID LP error RP {
        matchError(@3.first_line, "Error in func args", $$);
    }
    | Exp LB Exp RB {
        struct TreeNode *child[4] = {$1, $2, $3, $4};
        $$ = matchRule("Exp", child, 4, @1.first_line);
    }
    | Exp LB error RB {
        matchError(@3.first_line, "Error in array index", $$);
    }
    | Exp DOT ID {
        struct TreeNode *child[3] = {$1, $2, $3};
        $$ = matchRule("Exp", child, 3, @1.first_line);
    }
    | ID {
        struct TreeNode *child[1] = {$1};
        $$ = matchRule("Exp", child, 1, @1.first_line);
    }
    | INT {
        struct TreeNode *child[1] = {$1};
        $$ = matchRule("Exp", child, 1, @1.first_line);
    }
    | FLOAT {
        struct TreeNode *child[1] = {$1};
        $$ = matchRule("Exp", child, 1, @1.first_line);
    }
    ;

Args : Exp COMMA Args {
        struct TreeNode *child[3] = {$1, $2, $3};
        $$ = matchRule("Args", child, 3, @1.first_line);
    }
    | Exp {
        struct TreeNode *child[1] = {$1};
        $$ = matchRule("Args", child, 1, @1.first_line);
    }
%%

void yyerror(const char *msg)
{
    return;
}

struct TreeNode * matchRule(char *father, struct TreeNode* child[], int num, int _lineno)
{
    int index = 0;
    for(int i = 0; i < num; ++i)
    {
        if(child[i] != NULL)
        {    
            child[index] = child[i];
            ++index;
        }
    }
    if(index == 0)
        return NULL;
    struct TreeNode * p = (struct TreeNode *)malloc(sizeof(struct TreeNode));
    memset((void *)p, 0, sizeof(struct TreeNode));
    for(int i = 0; i < index - 1; ++i)
        child[i]->next = child[i + 1]; 
    child[index - 1]->next = NULL;
    p->child = child[0];
    p->lineno = _lineno;
    strcpy(p->name, father);
    return p;
}

void printToken(struct TreeNode* p)
{
    if(strcmp(p->name, "TYPE") == 0)
        printf("%s: %s\n", p->name, p->value.type_str);
    else if(strcmp(p->name, "ID") == 0)
        printf("%s: %s\n", p->name, p->value.type_str);
    else if(strcmp(p->name, "INT") == 0)
        printf("%s: %d\n", p->name, p->value.type_int);
    else if(strcmp(p->name, "FLOAT") == 0)
        printf("%s: %f\n", p->name, p->value.type_float);
    else
        printf("%s\n", p->name);

}

void printTree(struct TreeNode *p, int level)
{
    for(int i = 0; i < level * 2; ++i)
        putc(' ', stdout);
    struct TreeNode* c = p->child;

    if(c != NULL)
        printf("%s (%d)\n", p->name, p->lineno);
    else
        printToken(p);
        
    
    while(c != NULL)
    {
        printTree(c, level + 1);
        c = c->next;
    }
}

void freeTree(struct TreeNode *p)
{
    if(p == NULL)
        return;
    struct TreeNode * c = p->child, *temp;
    while(c != NULL)
    {
        temp = c;
        c = c->next;
        freeTree(temp);
    }
    free(p);
}
