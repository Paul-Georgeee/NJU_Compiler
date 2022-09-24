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
{STRUCT} {printf("STRUCT\n");}
{RETURN} {printf("RETURN\n");}
{IF} {printf("IF\n");}
{ELSE} {printf("ELSE\n");}
{WHILE} {printf("WHILE\n");}
{TYPE} {printf("TYPE\n");}
(({digit_z}{digit}*|0)\.{digit}+)|({digit}*\.{digit}*[Ee][+-]?{digit}+) {printf("float num %s \n", yytext);}
0[0-8]+ {printf("octal num %s\n", yytext);}
0x[0-9a-fA-F]+ {printf("hexadecimal num %s\n", yytext);}
{digit_z}{digit}*|0 {printf("decimal num %d \n", atoi(yytext));}
{letter}({letter}|{digit})* {printf("ID %s\n", yytext);}
{SEMI} {printf("SEMI\n");}
{COMMA} {printf("COMMA\n");}
{ASSIGNOP} {printf("ASSIGNOP\n");}
{RELOP} {printf("RELOP\n");}
{PLUS} {printf("PLUS\n");}
{MINUS} {printf("MINUS\n");}
{STAR} {printf("STAR\n");}
{DIV} {printf("DIV\n");}
{AND} {printf("AND\n");}
{OR} {printf("OR\n");}
{DOT} {printf("DOT\n");}
{NOT} {printf("NOT\n");}
{LP} {printf("LP\n");}
{RP} {printf("RP\n");}
{LB} {printf("LB\n");}
{RB} {printf("RB\n");}
{LC} {printf("LC\n");}
{RC} {printf("RC\n");}

" " {;}
. {printf("error\n");}
%%