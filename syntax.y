%{
    #include "lex.yy.c"
    #define YYERROR_VERBOSE

    void printError(int line, int col, char *descprition)
    {
        printf("Error Type B at Line %d Col %d: %s\n", line, col, descprition);
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

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%
Program : ExtDeflist
    ;
ExtDeflist : ExtDef ExtDeflist
    | %empty
    ;
ExtDef : Specifier ExtDecList SEMI
    | Specifier ExtDecList error {printError(@3.first_line, @3.first_column, "Miss \";\" in ExtDef");}
    | Specifier error SEMI {printError(@2.first_line, @2.first_column, "Error in ExtDef");}
    | Specifier SEMI
    | Specifier error {printError(@2.first_line, @2.first_column, "Miss \";\" in ExtDef");}
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
    | VarDec LB error RB {printError(@1.first_line, @1.first_column, "ERROR in VarDec");}
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
    ;
StmtList : Stmt StmtList
    | %empty
    ;
Stmt : Exp SEMI
    | Exp error {printError(@2.first_line, @2.first_column, "Miss \";\" after Exp");}
    | CompSt
    
    | RETURN Exp SEMI
    | RETURN error {printError(@2.first_line, @2.first_column, "Miss something after return");}
    
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE
    | IF error RP Stmt %prec LOWER_THAN_ELSE {printError(@2.first_line, @2.first_column, "Miss \"(\" after if");}
    | IF LP Exp error Stmt %prec LOWER_THAN_ELSE {printError(@4.first_line, @4.first_column, "Miss \")\" after if");} 
    | IF Exp error Stmt %prec LOWER_THAN_ELSE {printError(@3.first_line, @3.first_column, "Miss \"()\" after if");}
    | IF LP error RP Stmt %prec LOWER_THAN_ELSE {printError(@3.first_line, @3.first_column, "Error in if exp");}
   
    | IF LP Exp RP Stmt ELSE Stmt
    | IF error RP Stmt ELSE Stmt {printError(@2.first_line, @2.first_column, "Miss \"(\" after if");}
    | IF LP Exp error Stmt ELSE Stmt {printError(@4.first_line, @4.first_column, "Miss \")\" after if");} 
    | IF Exp error Stmt ELSE Stmt {printError(@3.first_line, @3.first_column, "Miss \"()\" after if");}
    | IF LP error RP Stmt ELSE Stmt {printError(@3.first_line, @3.first_column, "Error in if exp");}
    | ELSE error {printError(@2.first_line, @2.first_column, "unmatch if else");}
    
    | WHILE LP Exp RP Stmt
    | WHILE error RP {printError(@2.first_line, @2.first_column, "Miss \"(\" after while");}
    | WHILE LP Exp error {printError(@4.first_line, @4.first_column, "Miss \")\" after while");} 
    | WHILE Exp error {printError(@3.first_line, @3.first_column, "Miss \"()\" after while");}
    | WHILE LP error RP {printError(@3.first_line, @3.first_column, "Error in while exp");}
    ;

DefList : Def DefList
    | %empty
    ;
Def : Specifier DecList SEMI
    | Specifier DecList error {printError(@3.first_line, @3.first_column, "Miss \";\" in Def");}
    | Specifier error SEMI {printError(@2.first_line, @3.first_column, "Error in Def");}
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
    | Exp LB error RB {printError(@3.first_line, @3.first_column, "Error in \"]\"");}

    | Exp DOT ID
    | ID
    | INT {}
    | FLOAT {}
    | Exp Exp error {printError(@2.first_line, @2.first_column, "Error in Exp");}
    ;
Args : Exp COMMA Args
    | Exp
%%

// void yyerror(char *msg)
// {
//     if(strcmp("syntax error", msg) != 0)
//         fprintf(stderr, "%s\n", msg);
// }