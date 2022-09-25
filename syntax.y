%{
    #include "lex.yy.c"
    #define YYERROR_VERBOSE
    #define YYDEBUG 1

    void printError(int line, int col, char *s)
    {
        printf("Error Type B at Line %d Col %d: Missing \"%s\"\n", line, col, s);
    }
%}
%union{
    int type_int;
    float type_float;
    double type_double;
}

%token INT 
%token FLOAT
%token STRUCT
%token RETURN IF ELSE WHILE TYPE
%token ID LC RC
%token COMMA SEMI
%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT
%left LP RP LB RB DOT
%%
Program : ExtDeflist
    ;
ExtDeflist : ExtDef ExtDeflist
    | %empty
    ;
ExtDef : Specifier ExtDecList SEMI
    | Specifier SEMI
    | Specifier FunDec CompSt
    ;
ExtDecList : VarDec
    | VarDec COMMA ExtDecList
    ;
Specifier : TYPE
    | StructSpecifier
    ;
StructSpecifier : STRUCT OptTag LC DefList RC
    | STRUCT Tag
    ;
OptTag : ID
    | %empty
    ;
Tag : ID
    ;

VarDec : ID
    | VarDec LB INT RB
    ;
FunDec : ID LP VarList RP
    | ID LP RP
    ;
VarList : ParamDec COMMA VarList
    | ParamDec
    ;
ParamDec : Specifier VarDec
    ;

CompSt : LC DefList StmtList RC
    | error RC {printError(@1.first_line, @1.first_column, "}");}
    ;
StmtList : Stmt StmtList
    | %empty
    ;
Stmt : Exp SEMI
    | CompSt
    | RETURN Exp SEMI
    | IF LP Exp RP Stmt
    | IF LP Exp RP Stmt ELSE Stmt
    | WHILE LP Exp RP Stmt
    | error SEMI {printError(@1.first_line, @2.first_column, ";");}
    ;

DefList : Def DefList
    | %empty
    ;
Def : Specifier DecList SEMI
    ;
DecList : Dec
    | Dec COMMA DecList
    ;
Dec : VarDec
    | VarDec ASSIGNOP Exp
    ;


Exp : Exp ASSIGNOP Exp {}
    | Exp AND Exp
    | Exp OR Exp
    | Exp RELOP Exp
    | Exp PLUS Exp {}
    | Exp MINUS Exp {}
    | Exp STAR Exp {}
    | Exp DIV Exp {}
    | LP Exp RP {}
    | MINUS Exp
    | NOT Exp
    | ID LP Args RP
    | ID LP RP
    | Exp LB Exp RB
    | Exp DOT ID
    | ID
    | INT {}
    | FLOAT {}
    | error RP {printError(@1.first_line, @1.last_column, ")");}
    ;
Args : Exp COMMA Args
    | Exp
%%

// void yyerror(char *msg)
// {
//     if(strcmp("syntax error", msg) != 0)
//         fprintf(stderr, "%s\n", msg);
// }