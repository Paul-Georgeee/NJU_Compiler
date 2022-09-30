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

    struct TreeNode * init_node(char * name);

    #define INITTOKEN_NOVAL(TOKENNAME)\
    {\
        has_error = 0;\
        yylval = init_node(#TOKENNAME);\
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

{STRUCT} { INITTOKEN_NOVAL(STRUCT)}
{RETURN} { INITTOKEN_NOVAL(RETURN)}
{IF} { INITTOKEN_NOVAL(IF)}
{ELSE} { INITTOKEN_NOVAL(ELSE)}
{WHILE} { INITTOKEN_NOVAL(WHILE)}
{TYPE} {
    has_error = 0;
    yylval = init_node("TYPE");
    sscanf(yytext, "%s", yylval->value.type_str);
    return TYPE;
    }
(({digit_z}{digit}*|0)\.{digit}+)|((({digit}+\.{digit}*)|({digit}*\.{digit}+))[Ee][+-]?{digit}+) {
    has_error = 0;
    yylval = init_node("FLOAT");
    sscanf(yytext, "%f", &(yylval->value.type_float));
    return FLOAT;
}
0[0-7]+ {
    has_error = 0;  
    yylval = init_node("INT");
    sscanf(yytext, "%o", &(yylval->value.type_int));    
    return INT;
    }
0x[0-9a-fA-F]+ {
    has_error = 0;
    yylval = init_node("INT");
    sscanf(yytext, "%x", &(yylval->value.type_int));      
    return INT;
    }
{digit_z}{digit}*|0 {
    has_error = 0;
    yylval = init_node("INT");
    sscanf(yytext, "%d", &(yylval->value.type_int));     
    return INT;
    }
{letter}({letter}|{digit})* {
    has_error = 0;
    yylval = init_node("ID");
    sscanf(yytext, "%s", yylval->value.type_str);
    return ID;
    }
{SEMI} { INITTOKEN_NOVAL(SEMI)}
{COMMA} { INITTOKEN_NOVAL(COMMA)}
{ASSIGNOP} { INITTOKEN_NOVAL(ASSIGNOP)}
{RELOP} { INITTOKEN_NOVAL(RELOP)}
{PLUS} { INITTOKEN_NOVAL(PLUS)}
{MINUS} { INITTOKEN_NOVAL(MINUS)}
{STAR} { INITTOKEN_NOVAL(STAR)}
{DIV} { INITTOKEN_NOVAL(DIV)}
{AND} { INITTOKEN_NOVAL(AND)}
{OR} { INITTOKEN_NOVAL(OR)}
{DOT} { INITTOKEN_NOVAL(DOT)}
{NOT} { INITTOKEN_NOVAL(NOT)}
{LP} { INITTOKEN_NOVAL(LP)}
{RP} { INITTOKEN_NOVAL(RP)}
{LB} { INITTOKEN_NOVAL(LB)}
{RB} { INITTOKEN_NOVAL(RB)}
{LC} { INITTOKEN_NOVAL(LC)}
{RC} { INITTOKEN_NOVAL(RC)}
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

struct TreeNode * init_node(char * name)
{
    struct TreeNode *p = (struct TreeNode *)malloc(sizeof(struct TreeNode));
    memset((void *)p, 0, sizeof(struct TreeNode));
    strcpy(p->name, name);
    return p;
}