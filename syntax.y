%{
    #define YYERROR_VERBOSE
    void yyerror(const char *msg);
    #define YYSTYPE struct TreeNode*

    int wrong = 0;

    struct TreeNode{
        char name[32];
        union{
            int type_int;
            float type_float;
            char type_str[32];
        } value;
        int lineno;
        struct TreeNode * child;
        struct TreeNode * next;
    };

    struct TreeNode * root;
    struct TreeNode * matchRule(char *father, struct TreeNode* child[], int num, int _lineno);

    #include "lex.yy.c"
    
    void printError(int line, int col, char *descprition)
    {
        wrong = 1;
        fprintf(stderr, "Error type B at Line %d: %s\n", line, descprition);
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
        printError(@2.first_line, @2.first_column, "Miss \";\" in ExtDef");
        $$ = NULL;
        YYERROR;
    }
    
    | Specifier SEMI {
        struct TreeNode * child[2] = {$1, $2};
        $$ = matchRule("ExtDef", child, 2, @1.first_line);
    }
    | Specifier %prec MISSERROR {
        printError(@1.first_line, @1.first_column, "Miss \";\" in ExtDef");
        $$ = NULL;
        YYERROR;
    }
    
    | Specifier FunDec CompSt {
        struct TreeNode * child[3] = {$1, $2, $3};
        $$ = matchRule("ExtDef", child, 3, @1.first_line);
    }

    | error SEMI {wrong = 1;$$ = NULL;}
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
        printError(@1.first_line, @1.first_column, "ERROR in VarDec");
        $$ = NULL;
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
        printError(@3.first_line, @3.first_column, "Error in Func Args");
        $$ = NULL;
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
        printError(@1.first_line, @1.first_column, "Miss \";\" in Exp statement");
        $$ = NULL;
        YYERROR;
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
        printError(@1.first_line, @1.first_column, "Miss \";\" in Return statement");
        $$ = NULL;
        YYERROR;
    }
    
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {
        struct TreeNode *child[5] = {$1, $2, $3, $4, $5};
        $$ = matchRule("Stmt", child, 5, @1.first_line);   
    }
    | IF error RP Stmt %prec LOWER_THAN_ELSE {
        printError(@3.first_line, @3.first_column, "Error in if statement");
        $$ = NULL;
    }
    | IF LP error  Stmt %prec LOWER_THAN_ELSE {
        printError(@3.first_line, @3.first_column, "Error in if statement");
        $$ = NULL;
    }
    
    | IF LP Exp RP Stmt ELSE Stmt {
        struct TreeNode *child[7] = {$1, $2, $3, $4, $5, $6, $7};
        $$ = matchRule("Stmt", child, 7, @1.first_line);  
    }
    | IF error RP Stmt ELSE Stmt {
        printError(@3.first_line, @3.first_column, "Error in if statement");
        $$ = NULL;
    }
    | IF LP error Stmt ELSE Stmt {
        printError(@3.first_line, @3.first_column, "Error in if statement");
        $$ = NULL;
    }

    | ELSE error {
        printError(@2.first_line, @2.first_column, "unmatch if else");
        $$ = NULL;
    }
    
    | WHILE LP Exp RP Stmt 
    {
        struct TreeNode *child[5] = {$1, $2, $3, $4, $5};
        $$ = matchRule("Stmt", child, 5, @1.first_line); 
    }
    | WHILE error RP Stmt {
        printError(@3.first_line, @3.first_column, "Error in while");
        $$ = NULL;    
    }
    | WHILE LP error Stmt {
        printError(@3.first_line, @3.first_column, "Error in while");
        $$ = NULL;
    }

    | STAR  %prec MISSERROR {
        printError(@1.first_line, @1.first_column, "Error in statement");
        $$ = NULL;
        YYERROR;
    }

    


    | error SEMI {wrong = 1;$$ = NULL;}
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
        printError(@1.first_line, @1.first_column, "Error in def");
        $$ = NULL;
        YYERROR;
        }
    | error SEMI {wrong = 1;$$ = NULL;}
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
    
    | MINUS Exp {
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

    | Exp LB Exp RB {
        struct TreeNode *child[4] = {$1, $2, $3, $4};
        $$ = matchRule("Exp", child, 4, @1.first_line);
    }
    | Exp LB error RB {
        printError(@3.first_line, @3.first_column, "Error in array index");
        $$ = NULL;
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

    /*| Exp ID {printError(@2.first_line, @2.first_column, "miss operator beween exp");}*/
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