%{
    #include "lex.yy.c"
%}
%union{
    int type_int;
    float type_float;
    double type_double;
}

%token <type_int>INT 
%token <type_float>FLOAT
%token STRUCT
%token ID LC RC
%token COMMA
%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT
%left LP RP LB RB DOT
%type <type_double> Exp
%%
Exp : Exp ASSIGNOP Exp {printf("use assign\n");}
    | Exp AND Exp
    | Exp OR Exp
    | Exp RELOP Exp
    | Exp PLUS Exp {printf("use add\n");}
    | Exp MINUS Exp {printf("use sub\n");}
    | Exp STAR Exp {printf("use mul\n");}
    | Exp DIV Exp {printf("use div\n");}
    | LP Exp RP {printf("use ()\n");}
    | MINUS Exp
    | NOT Exp
    | ID LP Args RP
    | ID LP RP
    | Exp LB Exp RB
    | Exp DOT ID
    | ID
    | INT {printf("use int\n");}
    | FLOAT {printf("use float\n");}
    | error RP
    ;
Args : Exp COMMA Args
    | Exp
%%
