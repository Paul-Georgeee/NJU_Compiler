#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "tree.h"
#include "semantic.h"
#define HASHSIZE 0x3fff

struct Symbol *nowFunc = NULL;
struct SymbolStack symbolStack;
struct Symbol hashtable[HASHSIZE] = {};
struct SymbolStack s;

struct Type _int = {BASIC, T_INT}, _float = {BASIC, T_FLOAT};
void freeSymbol(struct Symbol *s)
{
    memset((void *)s, 0, sizeof(struct Symbol));
    switch (s->flag)
    {
    case S_VAR:
        freeType(s->var);
        break;
    case S_FUNC:
        freeType(s->func.returnType);
        freeFieldList(s->func.args, 1);
        break;
    case S_STRUCT:
        freeFieldList(s->structure, 1);
        break;
    default:
        break;
    }
    free(s);
}

struct Symbol *getStackTop()
{
    return symbolStack.stack[symbolStack.tail];
}

void push()
{
    symbolStack.tail = symbolStack.tail + 1;
    symbolStack.stack[symbolStack.tail] = NULL;
}

void pop()
{
    struct Symbol * s = symbolStack.stack[symbolStack.tail];
    symbolStack.tail = symbolStack.tail - 1;
    while(s != NULL)
    {
        struct Symbol *tmp = s;
        s = s->blockLinkNext;
        tmp->hashLinkbefore->hashLinkNext = tmp->hashLinkNext;
        if(tmp->hashLinkNext != NULL)
            tmp->hashLinkNext->hashLinkbefore = tmp->hashLinkbefore;
    
        freeSymbol(tmp);
    }
}



struct Type * copyType(struct Type *t)
{
    assert(t->kind != ARRAY);
    if(t->kind == BASIC)
        return t;

    struct Type * ret = (struct Type *)malloc(sizeof(struct Type));
    memcpy(ret, t, sizeof(struct Type));
    return ret;
}

void semanticError(int type, int line, char *description)
{
    printf("Error type %d at Line %d: %s\n", type, line, description);
}

int checkFieldEqual(struct FieldList *f1, struct FieldList *f2)
{
    while(f1 != NULL && f2 != NULL)
    {
        if(checkTypeEqual(f1->type, f2->type) == 0)
            return 0;
        f1 = f1->next;
        f2 = f2->next;
    }
    if(f1 != NULL || f2 != NULL)
        return 0;

    return 1;
}

int checkTypeEqual(struct Type *t1, struct Type *t2)
{
    if(t1 == NULL || t2 == NULL)
        return 1;
    if(t1->kind != t2->kind)
        return 0;
    
    if(t1->kind == BASIC)
        return t1->basic == t2->basic;
    else if(t1->kind == ARRAY)
        return checkTypeEqual(t1->array.elem, t2->array.elem);   
    else 
    {
        // if(t1->structure.name == NULL || t2->structure.name == NULL)
        //     return 0;
        // return strcmp(t1->structure.name, t2->structure.name) == 0;
        return checkFieldEqual(t1->structure.field, t2->structure.field);
    }
}

void freeFieldList(struct FieldList *f, int freeTypeFlag)
{
    if(f == NULL)
        return;
    struct FieldList *tmp;
    while(f != NULL)
    {
        tmp = f;
        f = f->next;
        
        if(freeTypeFlag == 1)
            freeType(tmp->type);

        free(tmp);
    }
}

void freeType(struct Type *t)
{
    if(t == NULL)
        return;
    if(t->kind == BASIC)
        return;
    else if(t->kind == STRUCTURE)
    {
        free(t);
        return;
    }
    else
    {
        freeType(t->array.elem);
        free(t);
        return;
    }
}

void checkFuncArgs(struct TreeNode * practicalArgs, struct FieldList* formalArgs)
{
    int flag = strcmp(practicalArgs->name, "Args");

    //one doesn't have args, but one has
    if((formalArgs != NULL &&  flag != 0) || (formalArgs == NULL && flag ==0))
    {
        semanticError(9, practicalArgs->lineno, "The number of practical args an formal agrs does not match");
        return;
    }
    //both have 0 arg
    else if(flag != 0 && formalArgs == NULL)
        return;

    //get practical agrs
    struct FieldList *tmp1, *tmp2 = NULL;
    struct TreeNode *p = practicalArgs;
    while(p != NULL)
    {
        struct TreeNode *exp = p->child;
        traverseForExp(exp);
        tmp1 = (struct FieldList*)malloc(sizeof(struct FieldList));
        tmp1->next = tmp2;
        tmp1->type = exp->value.typeForExp;
        tmp2 = tmp1;
        if(exp->next != NULL)
            p = exp->next->next;
        else
            p = NULL;
    }

    //check whether the type is equal
    struct FieldList *f1 = formalArgs;
    while(f1 != NULL && tmp1 != NULL)
    {
        if(checkTypeEqual(f1->type, tmp1->type) == 0)
        {
            semanticError(9, practicalArgs->lineno, "The type of formal args doesn't match with practical type");
            freeFieldList(tmp2, 0);
            return;
        }
        f1 = f1->next;
        tmp1 = tmp1->next;
    }
    //the number of args is not equal
    if(f1 != NULL || tmp1 != NULL)
        semanticError(9, practicalArgs->lineno, "The number of practical args an formal agrs does not match"); 
    freeFieldList(tmp2, 0);
    return;
}   


//Find a member named "name" in struct field f
struct Type * checkStructField(struct FieldList *f, char *name)
{
    if(name == NULL)
        return NULL;
    while(f != NULL)
    {
        if(strcmp(f->name, name) == 0)
            return f->type;
        f = f->next;
    }
    return NULL;
}

unsigned int symbolHash(char *name)
{
    unsigned int val = 0, i;
    for (; *name; ++name)
    {
        val = (val << 2) + *name;
        if (i = val & (~HASHSIZE))
            val = (val ^ (i >> 22)) & HASHSIZE;
    }
    return val;
}

//Giving type, return the symbol of var which p points to
struct Symbol *traverseForVarDec(struct TreeNode *p, struct Type *t)
{
    assert(strcmp("VarDec", p->name) == 0);
    struct TreeNode *vardec = p;
    struct Type *tmp1 = t, *tmp2 = t;

    //handle the array type
    while (strcmp(vardec->child->name, "VarDec") == 0)
    {
        tmp1 = (struct Type *)malloc(sizeof(struct Type));
        tmp1->kind = ARRAY;
        tmp1->array.elem = tmp2;
        tmp1->array.size = vardec->child->next->next->value.type_int;
        tmp2 = tmp1;
        vardec = vardec->child;
    }

    assert(strcmp(vardec->child->name, "ID") == 0);
    struct Symbol *s = (struct Symbol *)malloc(sizeof(struct Symbol));
    s->flag = S_VAR;
    s->name = vardec->child->value.type_str;
    s->var = tmp1;
    
    return s;
}

//Var list for function formal args
//Return a list of the formal args
struct FieldList *traverseForVarList(struct TreeNode *p)
{
    assert(strcmp(p->name, "VarList") == 0);
    struct FieldList *tmp1, *tmp2 = NULL;
    while (p)
    {
        struct TreeNode *paramDec = p->child;
        struct Type *t = traverseForSpecifier(paramDec->child);

        if (paramDec->next != NULL)
            p = paramDec->next->next;
        else
            p = NULL;
        
        //Type is not exist and the formal arg is ignored
        if (t == NULL)
            continue;

        struct Symbol *s = traverseForVarDec(paramDec->child->next, t);
        //the arg name has exists
        if (insert(s) == -1)
        {
            semanticError(15, paramDec->lineno, "Var in func args is redefined");
            freeSymbol(s);
            continue;
        }

        tmp1 = (struct FieldList *)malloc(sizeof(struct FieldList));
        tmp1->next = tmp2;
        tmp1->name = s->name;
        tmp1->type = s->var;

        tmp2 = tmp1;
    }
    return tmp2;
}

//Struct field construct and return a list of the member
//Simalar to the func above, but "int a, b" could appear in struct member(while func args could not)
struct FieldList *formFieldlist(struct TreeNode *p) // p is a deflist
{
    assert(strcmp("DefList", p->name) == 0);
    struct FieldList *tmp1, *tmp2 = NULL;
    while (p != NULL)
    {
        struct TreeNode *def = p->child;
        struct Type *t = traverseForSpecifier(def->child);

        p = p->child->next;
        if (t == NULL)
            continue;

        struct TreeNode *declist = def->child->next;

        //Deal with such that "int a, b, c"
        //Note that every Var should have his own type(not share the same one expect basic type)
        //So use copyT which is copy from t
        while (declist)
        {
            struct Type *copyT = copyType(t);
            struct TreeNode *dec = declist->child;
            struct Symbol *s = traverseForVarDec(dec->child, copyT);

            if (dec->next != NULL)
                declist = dec->next->next;
            else
                declist = NULL;

            //member name has exist
            if (insert(s) == -1)
            {
                semanticError(15, dec->lineno, "Var in struct is redefined");
                freeSymbol(s);
                continue;
            }

            //initial struct member is not allowed
            if (dec->child->next != NULL)
                semanticError(15, dec->lineno, "Initial var in struct is invalid;");
            
            tmp1 = (struct FieldList *)malloc(sizeof(struct FieldList));
            tmp1->name = s->name;
            tmp1->type = s->var;
            tmp1->next = tmp2;
            tmp2 = tmp1;
        }
        freeType(t);
    }
    return tmp2;
}

//get the type and the defination of the struct
struct Type *traverseForSpecifier(struct TreeNode *p)
{
    struct Type *t = (struct Type *)malloc(sizeof(struct Type));
    //Basic Type
    if (strcmp(p->child->name, "TYPE") == 0)
    {
        if (strcmp("int", p->child->value.type_str) == 0)
            return &_int;
        else
            return &_float;
    }
    //Struct Type
    else
    {
        struct Type *t = (struct Type *)malloc(sizeof(struct Type));
        struct TreeNode *s = p->child; // StructSpecifier
        struct TreeNode *structTag = s->child->next;
        t->kind = STRUCTURE;

        //Just use a struct to define val such as "struct Test t;"
        //However it can also be a declaration of a struct, such as "struct Test;"
        //But here I don't handle with it
        
        if (strcmp(structTag->name, "Tag") == 0) 
        {
            //TODO()
            //it may be a declare of a struct

            // look up symbol table if the struct has been defined;
            struct TreeNode *structName = structTag->child;
            struct Symbol *symbol = search(structName->value.type_str);
            
            //not found
            if (symbol == NULL || symbol->flag != S_STRUCT)
            {
                semanticError(17, structTag->lineno, "Struct is not been defined");
                return NULL;
            }

            //I say that every one has its own type, 
            //but when it is a struct type, the struct field is shared
            //And it will be freed when freeing the struct symbol in symbol table 
            
            t->structure.field = symbol->structure;
            t->structure.name = symbol->name;
        }
        //Define a struct such as struct Test{int a;};
        else
        {
            int flag = strcmp(structTag->name, "OptTag");
            struct Symbol *symbol = (struct Symbol *)malloc(sizeof(struct Symbol));
            symbol->structure = NULL;
            //To handle the var scope. Because the sturct name and the sturct member name not in the same scope
            //So I first insert the struct name to symbol table then traverse the field list
            if(flag == 0)
            {
                symbol->name = structTag->child->value.type_str;
                if (insert(symbol) == -1)
                {
                    semanticError(16, structTag->lineno, "The name of the Struct has been use");
                    freeSymbol(symbol);
                    return NULL;
                }
            }

            push();
            struct TreeNode *deflist = flag == 0 ? structTag->next->next : structTag->next;
            struct FieldList *f = formFieldlist(deflist);
            if (flag == 0)
            {
                symbol->flag = S_STRUCT;
                symbol->structure = f;
                t->structure.name = structTag->child->name;
            }
            //When defining a anonymous struct(such as struct {int a;};),
            //it will not be inserted into symbol table.
            //May lead to memory leak here
            else
                t->structure.name = NULL;
            t->structure.field = f;
            pop();
        }
        return t;
    }
}

//handling the def of local var
void traverseForDefList(struct TreeNode *p)
{
    assert(strcmp("DefList", p->name) == 0);
    while (p != NULL)
    {
        struct TreeNode *def = p->child;
        struct Type *t = traverseForSpecifier(def->child);

        p = p->child->next;
        //TODO() 
        //Type of var is not defined. Should the var be inserted into symbol table?
        if (t == NULL)
            continue;

        struct TreeNode *declist = def->child->next;
        //handling such that "int a,b,c;" 
        while (declist)
        {
            //every var should have not point to the same struct Type,
            struct Type *copyT = copyType(t);

            struct TreeNode *dec = declist->child;
            struct Symbol *s = traverseForVarDec(dec->child, copyT);
            if (insert(s) == -1)
            {
                semanticError(3, dec->lineno, "The name of Var has been used");    
                freeSymbol(s);
            }
            else
            {
                //handling such that "int a = 10;"
                if (dec->child->next != NULL)
                {
                    struct TreeNode * exp = dec->child->next->next;
                    // check if exp is valid
                    traverseForExp(exp);
                    // check if the type match
                    if (checkTypeEqual(s->var, exp->value.typeForExp) == 0)
                        semanticError(5, dec->child->lineno, "The types on both sides of the equation do not match");
                }
            }
        

            if (dec->next != NULL)
                declist = dec->next->next;
            else
                declist = NULL;
        }

        freeType(t);

    }
}

//For a exp, if there is any error in it, its type is assigned NULL
//And if a exp's type is NULL, all the error will be ignored
//eg. exp -> exp1 + exp2. If exp1 has error, it's type will be NULL then exp 's type will be NULL, too
void traverseForExp(struct TreeNode *p)
{
    struct TreeNode *firstChild = p->child;
    //handling var or func
    if(strcmp(firstChild->name, "ID") == 0)
    {
        struct Symbol *s = search(firstChild->value.type_str);
        if(firstChild->next == NULL)    //var
        {
            p->value.leftFlag = 1;  // a var is a left var
            if(s == NULL)
            {
                semanticError(1, firstChild->lineno, "Var is not defined");
                p->value.typeForExp = NULL;
                return;
            }
            else if(s->flag != S_VAR)
            {
                semanticError(7, firstChild->lineno, "Struct name or func name can't be operator");
                p->value.typeForExp = NULL;
            }
            else
                p->value.typeForExp = s->var;
        }
        else        //func type
        {
            if(s == NULL)
            {
                semanticError(2, firstChild->lineno, "Func is not defined");
                p->value.typeForExp = NULL;
            }
            else if(s->flag != S_FUNC)
            {
                semanticError(11, firstChild->lineno, "Only func can be called");
                p->value.typeForExp = NULL;
            }
            else
            {
                checkFuncArgs(firstChild->next->next, s->func.args);
                p->value.typeForExp = s->func.returnType;
            }
        }
    }
    else if(strcmp(firstChild->name, "LP") == 0) //handle "(exp)"
    {
        traverseForExp(firstChild->next);
        p->value.leftFlag = firstChild->next->value.leftFlag;
        p->value.typeForExp = firstChild->next->value.typeForExp;
    }
    else if(strcmp(firstChild->name, "INT") == 0)
        p->value.typeForExp = &_int;
    else if(strcmp(firstChild->name, "FLOAT") == 0)
        p->value.typeForExp = &_float;
    else if(strcmp(firstChild->name, "Exp") == 0)
    {
        struct TreeNode *secondChild = firstChild->next;
        traverseForExp(firstChild);
        //struct field access
        if(strcmp(secondChild->name, "DOT") == 0)
        {
            struct Type *t = firstChild->value.typeForExp, *tmp = NULL;
            p->value.leftFlag = 1;   //it is a left val
            if(t == NULL)
            {
                p->value.typeForExp = NULL;
                return;
            }
            
            struct TreeNode *id = secondChild->next;
            if(t->kind != STRUCTURE)
                semanticError(13, firstChild->lineno, "Var is not a struct var");                   
            else if((tmp = checkStructField(t->structure.field, id->value.type_str)) == NULL)
                semanticError(14, id->lineno, "Access the inexisted member of the struct");
            
            p->value.typeForExp = tmp;
        }
        //array index
        else if(strcmp(secondChild->name, "LB") == 0)
        {
            struct TreeNode *arrayIndex = secondChild->next;
            struct Type *t = firstChild->value.typeForExp;
            p->value.leftFlag = 1;
            if(t == NULL)
            {
                p->value.typeForExp = NULL;
                return;
            }
            else if(t->kind != ARRAY) 
            {
                semanticError(10, firstChild->lineno, "Index operation is only used for array");
                p->value.typeForExp = NULL;
                return;
            }

            traverseForExp(arrayIndex);
            if(checkTypeEqual(arrayIndex->value.typeForExp, &_int) == 0) //check whether index is int
                semanticError(12, arrayIndex->lineno, "Index of the array should be INT");
            
            p->value.typeForExp = t->array.elem;
        }
        else //binary op(including assignop)
        {
            int flag = strcmp(secondChild->name, "ASSIGNOP");
            struct TreeNode *thirdChild = secondChild->next;  //another exp
            traverseForExp(thirdChild);

            if(firstChild->value.typeForExp == NULL || thirdChild->value.typeForExp == NULL)
            {
                p->value.typeForExp = NULL;
                return;
            }
            else if(checkTypeEqual(firstChild->value.typeForExp, thirdChild->value.typeForExp) == 0) //check whether type is equal
            {
                if(flag == 0)
                    semanticError(5, firstChild->lineno, "The type doesn't match near ASSIGNOP");
                else
                    semanticError(7, firstChild->lineno, "The type doesn't match near operator");
                p->value.typeForExp = NULL;
                return;
            }

            if(flag == 0) //ASSIGNOP
            {
                if(firstChild->value.leftFlag == 0)
                {
                    semanticError(6, firstChild->lineno, "The operator on the left side of the ASSIGNOP is right value");
                    p->value.typeForExp = NULL;
                    return;
                }
                p->value.typeForExp = firstChild->value.typeForExp;
            }
            else 
            {
                //operator can only used on basic type
                if(firstChild->value.typeForExp->kind != BASIC)
                {
                    semanticError(7, firstChild->lineno, "The type doesn't match near operator");
                    p->value.typeForExp = NULL;
                    return;
                }
                if(strcmp(secondChild->name, "OR") == 0 || strcmp(secondChild->name, "AND") == 0)
                {
                    //logic operator can only used on int
                    if(checkTypeEqual(firstChild->value.typeForExp, &_int) == 0)
                    {
                        semanticError(7, firstChild->lineno, "The type doesn't match near operator");
                        p->value.typeForExp = NULL;
                        return;
                    }
                    p->value.typeForExp = firstChild->value.typeForExp;
                }
                else if(strcmp(secondChild->name, "RELOP") == 0)
                    p->value.typeForExp = &_int;
                else
                    p->value.typeForExp = firstChild->value.typeForExp;

            }

        }

    }
    else //handling unary op
    {
        struct TreeNode *exp = firstChild->next;
        traverseForExp(exp);
        
        if(strcmp(firstChild->name, "MINUS") == 0)
        {
            if(checkTypeEqual(exp->value.typeForExp, &_int) == 0 && checkTypeEqual(exp->value.typeForExp, &_float) == 0)
            {
                semanticError(7, firstChild->lineno, "The operator of MINUS shouble be INT or FLOAT");
                p->value.typeForExp = NULL;
                return;
            }
        }
        else
        {
            if(checkTypeEqual(exp->value.typeForExp, &_int) == 0)
            {
                semanticError(7, firstChild->lineno, "The operator of NOT shouble be INT");
                p->value.typeForExp = NULL;
                return;
            }
        }
        p->value.typeForExp = exp->value.typeForExp;
    }

}

void traverseForStmt(struct TreeNode *p)
{
    struct TreeNode *firstChild = p->child;
    if (strcmp(firstChild->name, "Exp") == 0)
        traverseForExp(firstChild);
    else if (strcmp(firstChild->name, "RETURN") == 0)
    {
        traverseForExp(firstChild->next);
        if (checkTypeEqual(nowFunc->func.returnType, firstChild->next->value.typeForExp) == 0)
            semanticError(8, firstChild->next->lineno, "Return type does not match");
    }
    else if (strcmp(firstChild->name, "IF") == 0)
    {
        struct TreeNode *exp = firstChild->next->next, *firstStmt = exp->next->next;
        traverseForExp(exp);
        // check if condition type: only for int
        if (checkTypeEqual(exp->value.typeForExp, &_int) == 0)
            semanticError(7, exp->lineno, "If condition type should be int");
        traverseForStmt(firstStmt);
        if (firstStmt->next != NULL)
            traverseForStmt(firstStmt->next->next);
    }
    else if (strcmp(firstChild->name, "WHILE") == 0)
    {
        struct TreeNode *exp = firstChild->next->next, *firstStmt = exp->next->next;
        traverseForExp(exp);
        // check while condition type: only for int
        if (checkTypeEqual(exp->value.typeForExp, &_int) == 0)
            semanticError(7, exp->lineno, "While condition type should be int");
        traverseForStmt(firstStmt);
    }
    else if (strcmp(firstChild->name, "CompSt") == 0)
        traverseForCompSt(firstChild, 1);
}

//isPush: handly the scope. If isPush == 1, stack push
//Because if the CompSt is the one in "Specifier FunDec CompSt",
//then stack have pushed in handling "FunDec".(Doing this because that the args is in the same scope)
//If the CompSt is the one in "Stmt -> CompSt", it need push the stack
void traverseForCompSt(struct TreeNode *p, int isPush)
{
    if(isPush == 1)
        push();
    assert(strcmp(p->name, "CompSt") == 0);
    struct TreeNode *deflist = p->child->next;
    struct TreeNode *stmtlist = deflist->next;
    if (strcmp("DefList", deflist->name) != 0)
        stmtlist = deflist;
    else
        traverseForDefList(deflist);
    
    //may appear such that "CompSt -> LC RC"
    if (strcmp(stmtlist->name, "StmtList") == 0)
    {
        while (stmtlist != NULL)
        {
            traverseForStmt(stmtlist->child);
            stmtlist = stmtlist->child->next;
        }
    }
    if(isPush == 1)
        pop();
}

void traverseForExtDef(struct TreeNode *p)
{
    assert(strcmp("ExtDef", p->name) == 0);
    struct TreeNode *tmp = p->child->next, *specifier = p->child;
    struct Type *t = traverseForSpecifier(specifier);
    if(strcmp("ExtDecList", tmp->name) == 0)
    {
        //TODO():should insert into Symbol Table?
        //Here if we define a var which type has error, we don't insert it into symbol table
        if (t == NULL)
            return;
        while (tmp)
        {
            struct Type *copyT = copyType(t);
            struct TreeNode *vardec = tmp->child;
            struct Symbol *s = traverseForVarDec(vardec, copyT);
            
            if (insert(s) == -1)
            {
                semanticError(3, vardec->lineno, "The name of Var has been used");
                freeSymbol(s);
            }
            
            if (vardec->next != NULL)
                tmp = vardec->next->next;
            else
                tmp = NULL;
        }
        freeType(t);
    }
    else if (strcmp("FunDec", tmp->name) == 0)
    {
        struct Symbol *s = (struct Symbol *)malloc(sizeof(struct Symbol));
        s->name = tmp->child->value.type_str;
        s->flag = S_FUNC;
        s->func.returnType = t;

        struct Symbol *s1 = search(s->name); 
        //func has not been defined or declared
        //Here we insert the symbol before traverse the var list,
        //for that the args's scope is not the same with the func name
        if(s1 == NULL)
            insert(s);

        push();

        nowFunc = s; //nowFun is pointed to the func now we traverse, for check return type

        struct TreeNode *varlist = tmp->child->next->next;
        if (strcmp(varlist->name, "VarList") == 0)
            s->func.args = traverseForVarList(varlist);
        else //no args
            s->func.args = NULL;
        

        int flag = strcmp(tmp->next->name, "CompSt");
        s->func.hasDef = (flag == 0);

        //func has not been defined
        if(s1 == NULL)
            s->func.firstDeclareLine = tmp->lineno;
        else    //func has been defined or declared
        {
            nowFunc = s1;

            //Define a func more than 1 time.Or the name has been used
            if((s1->flag != S_FUNC)||(flag == 0 && s1->func.hasDef == 1))
                semanticError(4, tmp->lineno, "The name of Func has been used");
            else
            {
                //update that the func has been defined
                if(flag == 0)
                    s1->func.hasDef = 1;
                //check whether return type and args match
                if(checkTypeEqual(s1->func.returnType, s->func.returnType) == 0 || checkFieldEqual(s1->func.args, s->func.args) == 0)
                    semanticError(19, tmp->lineno, "The return type or the args in func declare and def don't match");
            }
            freeSymbol(s);
        }

        if(flag == 0)
            traverseForCompSt(tmp->next, 0);
        pop();
    }
    else
        return;
}

void traverse()
{   
    memset((void *)&s, 0, sizeof(struct SymbolStack));
    struct TreeNode *extDefList = root->child;
    while (extDefList != NULL)
    {
        traverseForExtDef(extDefList->child);
        extDefList = extDefList->child->next;
    }

    //After finishing traversing, check whether there is func just declared but not defined
    for(int i = 0; i < HASHSIZE; ++i)
    {
        struct Symbol *tmp = hashtable[i].hashLinkNext;
        while (tmp != NULL)
        {
            if(tmp->flag == S_FUNC && tmp->func.hasDef == 0)
                semanticError(18, tmp->func.firstDeclareLine, "Undefined function");
            tmp = tmp->hashLinkNext;
        }
        
    }
}

//Search in hash table
struct Symbol *search(char *name)
{
    unsigned int hashval = symbolHash(name);
    struct Symbol *s = hashtable[hashval].hashLinkNext;
    while (s != NULL)
    {
        if (strcmp(name, s->name) == 0)
            return s;
        s = s->hashLinkNext;
    }
    return NULL;
}

//Insert symbol into hash table
int insert(struct Symbol *s)
{
    struct Symbol *tmp = search(s->name);
    //Note that var name can't be same to the struct name
    //depth is used to represent the scope, so we check repeated name in the current scope
    if (tmp != NULL && (tmp->depth == symbolStack.tail || tmp->flag != s->flag))
        return -1;

    unsigned int hashval = symbolHash(s->name);
    //recode the scope depth
    s->depth = symbolStack.tail;

    //updtae doubly linked list
    if(hashtable[hashval].hashLinkNext != NULL)
        hashtable[hashval].hashLinkNext->hashLinkbefore = s;
    s->hashLinkNext = hashtable[hashval].hashLinkNext;
    hashtable[hashval].hashLinkNext = s;
    s->hashLinkbefore = &(hashtable[hashval]);

    //update the current scope list
    s->blockLinkNext = symbolStack.stack[symbolStack.tail];
    symbolStack.stack[symbolStack.tail] = s;

    return 0;
}


//For debug
void printFieldList(struct FieldList *f)
{
    if (f == NULL)
        return;
    printf("%s :", f->name);
    printType(f->type);
    printf(" ; ");
    printFieldList(f->next);
}

void printType(struct Type *t)
{
    if (t == NULL)
    {
        printf("NULL");
        return;
    }
    switch (t->kind)
    {
    case BASIC:
        if (t->basic == T_INT)
            printf(" Type: int.");
        else if (t->basic == T_FLOAT)
            printf(" Type: float.");
        else
            assert(0);
        break;
    case ARRAY:
        printf("[%d]", t->array.size);
        printType(t->array.elem);
        break;
    case STRUCTURE:
        if (t->structure.name != NULL)
            printf(" struct %s{", t->structure.name);
        else
            printf(" struct {");
        printFieldList(t->structure.field);
        printf("}");
        break;
    default:
        break;
    }
}
void printHashTable()
{
    for (int i = 0; i < HASHSIZE; ++i)
    {
        struct Symbol *tmp = hashtable[i].hashLinkNext;
        while (tmp != NULL)
        {
            switch (tmp->flag)
            {
            case S_VAR:
                printf("Define a Var named %s  ", tmp->name);
                printType(tmp->var);
                printf("\n");
                break;
            case S_FUNC:
                printf("Define a Func named %s with args ( ", tmp->name);
                printFieldList(tmp->func.args);
                printf(")\n return type is");
                printType(tmp->func.returnType);
                printf("\n");
                break;
            case S_STRUCT:
                printf("Define a struct named %s with fieldList {", tmp->name);
                printFieldList(tmp->structure);
                printf("}\n");
                break;
            default:
                break;
            }
            tmp = tmp->hashLinkNext;
        }
    }
}