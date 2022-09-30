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
    yylval = init_node("STRUCT");
    return STRUCT;
}
{RETURN} {
    has_error = 0;
    yylval = init_node("RETURN");
    return RETURN;
    }
{IF} {
    has_error = 0;
    yylval = init_node("IF");
    return IF;
    }
{ELSE} {
    has_error = 0;
    yylval = init_node("ELSE");
    return ELSE;
    }
{WHILE} {
    has_error = 0;
    yylval = init_node("WHILE");
    return WHILE;
    }
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
{SEMI} {
    has_error = 0;
    yylval = init_node("SEMI");
    return SEMI;
    }
{COMMA} {
    has_error = 0;
    yylval = init_node("COMMA");
    return COMMA;
    }
{ASSIGNOP} {
    has_error = 0;
    yylval = init_node("ASSIGNOP");
    return ASSIGNOP;
    }
{RELOP} {
    has_error = 0;
    yylval = init_node("RELOP");
    return RELOP;
    }
{PLUS} {
    has_error = 0;
    yylval = init_node("PLUS");
    return PLUS;
    }
{MINUS} {
    has_error = 0;
    yylval = init_node("MINUS");
    return MINUS;
    }
{STAR} {
    has_error = 0;
    yylval = init_node("STAR");
    return STAR;
    }
{DIV} {
    has_error = 0;    
    yylval = init_node("DIV");
    return DIV;
    }
{AND} {
    has_error = 0;
    yylval = init_node("AND");
    return AND;
    }
{OR} {
    has_error = 0;
    yylval = init_node("OR");
    return OR;
    }
{DOT} {
    has_error = 0;
    yylval = init_node("DOT");
    return DOT;
    }
{NOT} {
    has_error = 0;
    yylval = init_node("NOT");
    return NOT;
    }
{LP} {
    has_error = 0;
    yylval = init_node("LP");
    return LP;
    }
{RP} {
    has_error = 0;
    yylval = init_node("RP");
    return RP;
    }
{LB} {
    has_error = 0;
    yylval = init_node("LB");
    return LB;
    }
{RB} {
    has_error = 0;
    yylval = init_node("RB");
    return RB;
    }
{LC} {
    has_error = 0;
    yylval = init_node("LC");
    return LC;
    }
{RC} {
    has_error = 0;
    yylval = init_node("RC");
    return RC;
    }
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