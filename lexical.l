%{
    #include "syntax.tab.h"
    int has_error = 0;
    int yycolumn = 1;
    
    // #define YY_USER_ACTION \
    //     yylloc.first_line = yylloc.last_line = yylineno;\
    //     yylloc.first_column = yycolumn;\
    //     yylloc.last_column = yycolumn + yyleng - 1;\
    //     yycolumn += yyleng;
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
{STRUCT} {
    has_error = 0;
    printf("STRUCT\n");
    }
{RETURN} {
    has_error = 0;
    printf("RETURN\n");
    }
{IF} {
    has_error = 0;
    printf("IF\n");
    }
{ELSE} {
    has_error = 0;
    printf("ELSE\n");
    }
{WHILE} {
    has_error = 0;
    printf("WHILE\n");
    }
{TYPE} {
    has_error = 0;
    printf("TYPE\n");
    }
(({digit_z}{digit}*|0)\.{digit}+)|({digit}*\.{digit}*[Ee][+-]?{digit}+) {
    has_error = 0;
    sscanf(yytext, "%f", &(yylval.type_float));
    return FLOAT;
}
0[0-8]+ {
    has_error = 0;
    sscanf(yytext, "%o", &(yylval.type_int));
    return INT;
    }
0x[0-9a-fA-F]+ {
    has_error = 0;
    sscanf(yytext, "%x", &(yylval.type_int));
    return INT;
    }
{digit_z}{digit}*|0 {
    has_error = 0;
    sscanf(yytext, "%d", &(yylval.type_int));
    return INT;
    }
{letter}({letter}|{digit})* {
    has_error = 0;
   // printf("ID %s\n", yytext);
    return ID;
    }
{SEMI} {
    has_error = 0;
    printf("SEMI\n");
    }
{COMMA} {
    has_error = 0;
    //printf("COMMA\n");
    return COMMA;
    }
{ASSIGNOP} {
    has_error = 0;
    // printf("ASSIGNOP\n");
    return ASSIGNOP;
    }
{RELOP} {
    has_error = 0;
    return RELOP;
    // printf("RELOP\n");
    }
{PLUS} {//printf("PLUS\n"); 
    has_error = 0;
    return PLUS;
    }
{MINUS} {//printf("MINUS\n");
    has_error = 0;
    return MINUS;
    }
{STAR} {//printf("STAR\n");
    has_error = 0;
    return STAR;
    }
{DIV} {//printf("DIV\n");
    has_error = 0;    
    return DIV;
    }
{AND} {
    has_error = 0;
    return AND;
    }
{OR} {
    has_error = 0;
    return OR;
    }
{DOT} {
    has_error = 0;
    return DOT;
    }
{NOT} {
    has_error = 0;
    return NOT;
    }
{LP} {
    has_error = 0;
    // printf("LP\n");
    return LP;
    }
{RP} {
    has_error = 0;
    // printf("RP\n");
    return RP;
    }
{LB} {
    has_error = 0;
    return LB;
    }
{RB} {
    has_error = 0;
    return RB;
    }
{LC} {
    has_error = 0;
    return LC;
    }
{RC} {
    has_error = 0;
    return RC;
    }

" " {has_error = 0;}
\n {yycolumn = 1;}
. {
    if(has_error == 0)
    {
        printf("Error type A at line %d: unexpected token %s\n", yylineno, yytext);
        has_error = 1;
    }
    printf("error\n");
    }
%%