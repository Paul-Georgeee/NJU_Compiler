%{
    #include "syntax.tab.h"
    extern int wrong;
    int has_error = 0;
    int yycolumn = 1;
    struct TreeNode;

    #define YY_USER_ACTION \
        yylloc.first_line = yylloc.last_line = yylineno;\
        yylloc.first_column = yycolumn;\
        yylloc.last_column = yycolumn + yyleng - 1;\
        yycolumn += yyleng;

    struct TreeNode * init_node(char * name, int lineno);

    #define INITTOKEN_NOVAL(TOKENNAME, LINENO)\
    {\
        has_error = 0;\
        yylval = init_node(#TOKENNAME, LINENO);\
        return TOKENNAME;\
    }
%}
digit [0-9]
nondigit [^0-9]
digit_z [1-9]
digit_o [0-7]
letter [_a-zA-Z]
SEMI ;
COMMA ,
ASSIGNOP =
RELOP >|<|>=|<=|==|!=
PLUS \+
MINUS -
STAR \*
DIV \/
AND &&
OR \|\|
DOT \.
NOT !
LP \(
RP \)
LB \[
RB \]
LC \{
RC \}
STRUCT struct
RETURN return
IF if
ELSE else
WHILE while
TYPE int|float

%option yylineno

%%

{STRUCT} { INITTOKEN_NOVAL(STRUCT, yylineno)}
{RETURN} { INITTOKEN_NOVAL(RETURN, yylineno)}
{IF} { INITTOKEN_NOVAL(IF, yylineno)}
{ELSE} { INITTOKEN_NOVAL(ELSE, yylineno)}
{WHILE} { INITTOKEN_NOVAL(WHILE, yylineno)}
{TYPE} {
    has_error = 0;
    yylval = init_node("TYPE", yylineno);
    sscanf(yytext, "%s", yylval->value.type_str);
    return TYPE;
    }
(({digit_z}{digit}*|0)\.{digit}+)|((({digit}+\.{digit}*)|({digit}*\.{digit}+))[Ee][+-]?{digit}+) {
    has_error = 0;
    yylval = init_node("FLOAT", yylineno);
    sscanf(yytext, "%f", &(yylval->value.type_float));
    return FLOAT;
}
0[0-7]+ {
    has_error = 0;  
    yylval = init_node("INT", yylineno);
    sscanf(yytext, "%o", &(yylval->value.type_int));    
    return INT;
    }
0x[0-9a-fA-F]+ {
    has_error = 0;
    yylval = init_node("INT", yylineno);
    sscanf(yytext, "%x", &(yylval->value.type_int));      
    return INT;
    }
{digit_z}{digit}*|0 {
    has_error = 0;
    yylval = init_node("INT", yylineno);
    sscanf(yytext, "%d", &(yylval->value.type_int));     
    return INT;
    }
{letter}({letter}|{digit})* {
    has_error = 0;
    yylval = init_node("ID", yylineno);
    sscanf(yytext, "%s", yylval->value.type_str);
    return ID;
    }
{RELOP} {
    has_error = 0;
    yylval = init_node("RELOP", yylineno);
    sscanf(yytext, "%s", yylval->value.type_str);
    return RELOP;
    }
{SEMI} { INITTOKEN_NOVAL(SEMI, yylineno)}
{COMMA} { INITTOKEN_NOVAL(COMMA, yylineno)}
{ASSIGNOP} { INITTOKEN_NOVAL(ASSIGNOP, yylineno)}
{PLUS} { INITTOKEN_NOVAL(PLUS, yylineno)}
{MINUS} { INITTOKEN_NOVAL(MINUS, yylineno)}
{STAR} { INITTOKEN_NOVAL(STAR, yylineno)}
{DIV} { INITTOKEN_NOVAL(DIV, yylineno)}
{AND} { INITTOKEN_NOVAL(AND, yylineno)}
{OR} { INITTOKEN_NOVAL(OR, yylineno)}
{DOT} { INITTOKEN_NOVAL(DOT, yylineno)}
{NOT} { INITTOKEN_NOVAL(NOT, yylineno)}
{LP} { INITTOKEN_NOVAL(LP, yylineno)}
{RP} { INITTOKEN_NOVAL(RP, yylineno)}
{LB} { INITTOKEN_NOVAL(LB, yylineno)}
{RB} { INITTOKEN_NOVAL(RB, yylineno)}
{LC} { INITTOKEN_NOVAL(LC, yylineno)}
{RC} { INITTOKEN_NOVAL(RC, yylineno)}
"//" {
    char c = input();
    while(c != '\n') {c = input();}
    yycolumn = 1;
    }
"/*" {
    char last = 0, now = input();
    while((last == '*' && now == '/') == 0)
    {
        last = now;
        now = input();
    }
}
\t {has_error = 0;}
" " {has_error = 0;}
\n {yycolumn = 1;}
. {
    wrong = 1;
    if(has_error == 0)
    {
        fprintf(stdout, "Error type A at line %d: unexpected token %s\n", yylineno, yytext);
        has_error = 1;
    }
    
}
%%

struct TreeNode * init_node(char * name, int lineno)
{
    struct TreeNode *p = (struct TreeNode *)malloc(sizeof(struct TreeNode));
    memset((void *)p, 0, sizeof(struct TreeNode));
    strcpy(p->name, name);
    p->lineno = lineno;
    return p;
}