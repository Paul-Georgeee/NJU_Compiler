%{
    #define YYERROR_VERBOSE
    void yyerror(const char *msg);
    #define YYSTYPE struct TreeNode*

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
    struct TreeNode * matchRule(char *father, struct TreeNode* child[], int num);

    #include "lex.yy.c"
    
    void printError(int line, int col, char *descprition)
    {
        printf("Error Type B at Line %d Col %d: %s\n", line, col, descprition);
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
        $$ = matchRule("Program", child, 1);
        root = $$;
    }
    ;
ExtDefList : ExtDef ExtDefList {
        struct TreeNode * child[2] = {$1, $2};
        if($2 == NULL)
            $$ = matchRule("ExtDefList", child, 1);
        else
            $$ = matchRule("ExtDefList", child, 2);
    }
    | %empty {$$ = NULL;}
    ;
ExtDef : Specifier ExtDecList SEMI {
        struct TreeNode * child[3] = {$1, $2, $3};
        $$ = matchRule("ExtDef", child, 3);
    }

    | Specifier ExtDecList %prec MISSERROR {
        printError(@2.first_line, @2.first_column, "Miss \";\" in ExtDef");
        YYERROR;
    }
    
    | Specifier SEMI {
        struct TreeNode * child[2] = {$1, $2};
        $$ = matchRule("ExtDef", child, 2);
    }
    | Specifier %prec MISSERROR {
        printError(@1.first_line, @1.first_column, "Miss \";\" in ExtDef");
        YYERROR;
    }
    
    | Specifier FunDec CompSt {
        struct TreeNode * child[3] = {$1, $2, $3};
        $$ = matchRule("ExtDef", child, 3);
    }

    | error SEMI {}
    ;

ExtDecList : VarDec {
        struct TreeNode * child[1] = {$1};
        $$ = matchRule("ExtDecList", child, 1);
    }
    | VarDec COMMA ExtDecList
    {
        struct TreeNode * child[3] = {$1, $2, $3};
        $$ = matchRule("ExtDecList", child, 3);
    }
    ;

Specifier : TYPE {
        struct TreeNode * child[1] = {$1};
        $$ = matchRule("Specifier", child, 1);
    }
    | StructSpecifier {
        struct TreeNode * child[1] = {$1};
        $$ = matchRule("Specifier", child, 1);
    }
    ;
StructSpecifier : STRUCT OptTag LC DefList RC {
        if($2 == NULL)
        {
            struct TreeNode * child[4] = {$1, $3, $4, $5};
            $$ = matchRule("StructSpecifier", child, 4);
        }
        else
        {
            struct TreeNode * child[5] = {$1, $2, $3, $4, $5};
            $$ = matchRule("StructSpecifier", child, 5);            
        }

    }
    | STRUCT Tag {
        struct TreeNode * child[2] = {$1, $2};
        $$ = matchRule("StructSpecifier", child, 2);
    }
    ;
OptTag : ID {
        struct TreeNode * child[1] = {$1};
        $$ = matchRule("OptTag", child, 1);
    }
    | %empty{
        $$ = NULL;
    }
    ;
Tag : ID {
        struct TreeNode * child[1] = {$1};
        $$ = matchRule("Tag", child, 1);
    }
    ;

VarDec : ID {
        struct TreeNode * child[1] = {$1};
        $$ = matchRule("VacDec", child, 1);
    }

    | VarDec LB INT RB {
        struct TreeNode * child[4] = {$1, $2, $3, $4};
        $$ = matchRule("VacDec", child, 4);
    }

    | VarDec LB error RB {printError(@1.first_line, @1.first_column, "ERROR in VarDec");}
    ;

FunDec : ID LP VarList RP {
        struct TreeNode * child[4] = {$1, $2, $3, $4};
        $$ = matchRule("FunDec", child, 4);
    }

    | ID LP RP {
        struct TreeNode * child[3] = {$1, $2, $3};
        $$ = matchRule("FunDec", child, 3);
    }

    | ID LP error RP {printError(@3.first_line, @3.first_column, "Error in Func Args");}
    ;

VarList : ParamDec COMMA VarList {
        struct TreeNode * child[3] = {$1, $2, $3};
        $$ = matchRule("VarList", child, 3);
    }

    | ParamDec {
        struct TreeNode * child[1] = {$1};
        $$ = matchRule("VarList", child, 1);
    }
    ;

ParamDec : Specifier VarDec {
        struct TreeNode *child[2] = {$1, $2};
        $$ = matchRule("ParamDec", child, 2);
    }
    ;

CompSt : LC DefList StmtList RC{
        struct TreeNode *child[4] = {$1, $2, $3, $4};
        $$ = matchRule("CompSt", child, 4);
    }
    ;

StmtList : Stmt StmtList {
        struct TreeNode *child[2] = {$1, $2};
        if($2 == NULL)
            $$ = matchRule("StmtList", child, 1);
        else
            $$ = matchRule("StmtList", child, 2);
    }

    | %empty {$$ = NULL;}
    ;
Stmt : Exp SEMI {
        struct TreeNode *child[2] = {$1, $2};
        $$ = matchRule("Stmt", child, 2);
    }

    | Exp %prec MISSERROR {
        printError(@1.first_line, @1.first_column, "Miss \";\" in Exp statement");
        YYERROR;
        }

    | CompSt {
        struct TreeNode *child[1] = {$1};
        $$ = matchRule("Compst", child, 1);
    }
    
    | RETURN Exp SEMI {
        struct TreeNode *child[3] = {$1, $2, $3};
        $$ = matchRule("Stmt", child, 3);    
    }
    | RETURN %prec MISSERROR {
        printError(@1.first_line, @1.first_column, "Miss \";\" in Return statement");
        YYERROR;
    }
    
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {
        struct TreeNode *child[5] = {$1, $2, $3, $4, $5};
        $$ = matchRule("Stmt", child, 5);   
    }
    | IF error RP Stmt %prec LOWER_THAN_ELSE {printError(@3.first_line, @3.first_column, "Error in if statement");}
    | IF LP error  Stmt %prec LOWER_THAN_ELSE {printError(@3.first_line, @3.first_column, "Error in if statement");}
    
    | IF LP Exp RP Stmt ELSE Stmt {
        struct TreeNode *child[7] = {$1, $2, $3, $4, $5, $6, $7};
        $$ = matchRule("Stmt", child, 7);  
    }
    | IF error RP Stmt ELSE Stmt {printError(@3.first_line, @3.first_column, "Error in if statement");}
    | IF LP error Stmt ELSE Stmt {printError(@3.first_line, @3.first_column, "Error in if statement");}

    | ELSE error {printError(@2.first_line, @2.first_column, "unmatch if else");}
    
    | WHILE LP Exp RP Stmt 
    {
        struct TreeNode *child[5] = {$1, $2, $3, $4, $5};
        $$ = matchRule("Stmt", child, 5); 
    }
    | WHILE error RP Stmt {printError(@3.first_line, @3.first_column, "Error in while");}
    | WHILE LP error Stmt {printError(@3.first_line, @3.first_column, "Error in while");}


    | error SEMI
    ;

DefList : Def DefList {
        struct TreeNode *child[2] = {$1, $2};
        if($2 == NULL)
            $$ = matchRule("DefList", child, 1);
        else
            $$ = matchRule("DefList", child, 2);
    }

    | %empty {$$ = NULL;}
    ;

Def : Specifier DecList SEMI {
        struct TreeNode *child[3] = {$1, $2, $3};
        $$ = matchRule("Def", child, 3);
    }
    | Specifier DecList %prec MISSERROR {
        printError(@1.first_line, @1.first_column, "Error in def");
        YYERROR;
        }
    | error SEMI 
    ;

DecList : Dec {
        struct TreeNode *child[1] = {$1};
        $$ = matchRule("DecList", child, 1);
    }
    | Dec COMMA DecList {
        struct TreeNode *child[3] = {$1, $2, $3};
        $$ = matchRule("DecList", child, 3);
    }
    ;

Dec : VarDec {
        struct TreeNode *child[1] = {$1};
        $$ = matchRule("Dec", child, 1);
    }
    
    | VarDec ASSIGNOP Exp {
        struct TreeNode *child[3] = {$1, $2, $3};
        $$ = matchRule("Dec", child, 3);
    }
    ;

Exp : Exp ASSIGNOP Exp {
        struct TreeNode *child[3] = {$1, $2, $3};
        $$ = matchRule("Exp", child, 3);
    }

    | Exp AND Exp {
        struct TreeNode *child[3] = {$1, $2, $3};
        $$ = matchRule("Exp", child, 3);
    }
    
    | Exp OR Exp {
        struct TreeNode *child[3] = {$1, $2, $3};
        $$ = matchRule("Exp", child, 3);
    }
    
    | Exp RELOP Exp {
        struct TreeNode *child[3] = {$1, $2, $3};
        $$ = matchRule("Exp", child, 3);
    }
    
    | Exp PLUS Exp {
        struct TreeNode *child[3] = {$1, $2, $3};
        $$ = matchRule("Exp", child, 3);
    }
    
    | Exp MINUS Exp {
        struct TreeNode *child[3] = {$1, $2, $3};
        $$ = matchRule("Exp", child, 3);
    }
    
    | Exp STAR Exp {
        struct TreeNode *child[3] = {$1, $2, $3};
        $$ = matchRule("Exp", child, 3);
    }
    
    | Exp DIV Exp {
        struct TreeNode *child[3] = {$1, $2, $3};
        $$ = matchRule("Exp", child, 3);
    }
    
    | LP Exp RP {
        struct TreeNode *child[3] = {$1, $2, $3};
        $$ = matchRule("Exp", child, 3);
    }
    
    | MINUS Exp {
        struct TreeNode *child[2] = {$1, $2};
        $$ = matchRule("Exp", child, 2);
    }
    
    | NOT Exp {
        struct TreeNode *child[2] = {$1, $2};
        $$ = matchRule("Exp", child, 2);
    }

    | ID LP Args RP  {
        struct TreeNode *child[4] = {$1, $2, $3, $4};
        $$ = matchRule("Exp", child, 4);
    }
    
    | ID LP RP {
        struct TreeNode *child[3] = {$1, $2, $3};
        $$ = matchRule("Exp", child, 3);
    }

    | Exp LB Exp RB {
        struct TreeNode *child[4] = {$1, $2, $3, $4};
        $$ = matchRule("Exp", child, 4);
    }
    | Exp LB error RB {printError(@3.first_line, @3.first_column, "Error in array index");}

    | Exp DOT ID {
        struct TreeNode *child[3] = {$1, $2, $3};
        $$ = matchRule("Exp", child, 3);
    }
    | ID {
        struct TreeNode *child[1] = {$1};
        $$ = matchRule("Exp", child, 1);
    }
    | INT {
        struct TreeNode *child[1] = {$1};
        $$ = matchRule("Exp", child, 1);
    }
    | FLOAT {
        struct TreeNode *child[1] = {$1};
        $$ = matchRule("Exp", child, 1);
    }

    /*| Exp ID {printError(@2.first_line, @2.first_column, "miss operator beween exp");}*/
    ;
Args : Exp COMMA Args {
        struct TreeNode *child[3] = {$1, $2, $3};
        $$ = matchRule("Args", child, 3);
    }
    | Exp {
        struct TreeNode *child[1] = {$1};
        $$ = matchRule("Args", child, 1);
    }
%%

// void yyerror(char *msg)
// {
//     if(strcmp("syntax error", msg) != 0)
//         fprintf(stderr, "%s\n", msg);
// }

struct TreeNode * matchRule(char *father, struct TreeNode* child[], int num)
{
    struct TreeNode * p = (struct TreeNode *)malloc(sizeof(struct TreeNode));
    for(int i = 0; i < num - 1; ++i)
        child[i]->next = child[i + 1]; 
    child[num - 1]->next = NULL;
    p->child = child[0];
    strcpy(p->name, father);
    return p;
}

void print(struct TreeNode *p)
{
    struct TreeNode* c = p->child;
    printf("%s\n", p->name);
    while(c != NULL)
    {
        print(c);
        c = c->next;
    }
}