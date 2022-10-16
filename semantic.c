#include "tree.h"
#include "semantic.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#define HASHSIZE 0x3fff

struct Symbol *nowFunc = NULL;

struct Symbol *hashtable[HASHSIZE] = {};
struct SymbolStack s;

struct Type _int = {BASIC, T_INT}, _float = {BASIC, T_FLOAT};

void semanticError(int type, int line, char *description)
{
    printf("Error type %d at Line %d: %s\n", type, line, description);
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
        return strcmp(t1->structure.name, t2->structure.name) == 0;
    
}

void freeFieldList(struct FieldList *f)
{
    struct FieldList *tmp;
    while(f != NULL)
    {
        tmp = f;
        f = f->next;
        free(tmp);
    }
}

void checkFuncArgs(struct TreeNode * practicalArgs, struct FieldList* formalArgs)
{
    int flag = strcmp(practicalArgs->name, "Args");
    if((formalArgs != NULL &&  flag != 0) || (formalArgs == NULL && flag ==0))
    {
        semanticError(9, practicalArgs->lineno, "The number of practical args an formal agrs does not match");
        return;
    }

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

    struct FieldList *f1 = formalArgs;
    while(f1 != NULL && tmp1 != NULL)
    {
        if(checkTypeEqual(f1->type, tmp1->type) == 0)
        {
            semanticError(9, practicalArgs->lineno, "The type of formal args doesn't match with practical type");
            freeFieldList(tmp2);
            return;
        }
        f1 = f1->next;
        tmp1 = tmp1->next;
    }
    if(f1 != NULL || tmp1 != NULL)
        semanticError(9, practicalArgs->lineno, "The number of practical args an formal agrs does not match"); 
    freeFieldList(tmp2);
    return;
}   

struct Type * checkStructField(struct FieldList *f, char *name)
{
    while(f != 0)
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

struct Symbol *traverseForVarDec(struct TreeNode *p, struct Type *t)
{
    assert(strcmp("VarDec", p->name) == 0);
    struct TreeNode *vardec = p;
    struct Type *tmp1 = t, *tmp2 = t;
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

        if (t == NULL)
            continue;

        struct Symbol *s = traverseForVarDec(paramDec->child->next, t);

        if (insert(s) == -1)
        {
            semanticError(15, paramDec->lineno, "Var in func args is redefined");
            free(s);
            continue;
        }
        tmp1 = (struct FieldList *)malloc(sizeof(struct FieldList));
        tmp1->next = tmp2;
        tmp1->name = s->name;
        tmp1->type = t;

        tmp2 = tmp1;
    }
    return tmp2;
}
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

        while (declist)
        {
            struct TreeNode *dec = declist->child;
            struct Symbol *s = traverseForVarDec(dec->child, t);

            if (dec->next != NULL)
                declist = dec->next->next;
            else
                declist = NULL;

            if (insert(s) == -1)
            {
                semanticError(15, dec->lineno, "Var in struct is redefined");
                free(s);
                continue;
            }
            if (dec->child->next != NULL)
            {
                semanticError(15, dec->lineno, "Initial var in struct is invalid;");
            }
            tmp1 = (struct FieldList *)malloc(sizeof(struct FieldList));
            tmp1->name = s->name;
            tmp1->type = s->var;
            tmp1->next = tmp2;
            tmp2 = tmp1;
        }
    }
    return tmp2;
}

struct Type *traverseForSpecifier(struct TreeNode *p)
{
    struct Type *t = (struct Type *)malloc(sizeof(struct Type));
    if (strcmp(p->child->name, "TYPE") == 0)
    {
        t->kind = BASIC;
        if (strcmp("int", p->child->value.type_str) == 0)
            t->basic = T_INT;
        else
            t->basic = T_FLOAT;
    }
    else
    {
        struct TreeNode *s = p->child; // StructSpecifier
        struct TreeNode *structTag = s->child->next;
        t->kind = STRUCTURE;
        if (strcmp(structTag->name, "Tag") == 0) // just def a struct val'
        {
            // look up symbol table if the struct has been defined;
            struct TreeNode *structName = structTag->child;
            struct Symbol *symbol = search(structName->value.type_str);
            if (symbol == NULL || symbol->flag != S_STRUCT)
            {
                semanticError(17, structTag->lineno, "Struct is not been defined");
                return NULL;
            }
            t->structure.field = symbol->structure;
            t->structure.name = symbol->name;
        }
        else
        {
            int flag = strcmp(structTag->name, "OptTag");
            struct TreeNode *deflist = flag == 0 ? structTag->next->next : structTag->next;
            struct FieldList *f = formFieldlist(deflist);
            if (flag == 0)
            {
                struct Symbol *symbol = (struct Symbol *)malloc(sizeof(struct Symbol));
                symbol->flag = S_STRUCT;
                symbol->name = structTag->child->value.type_str;
                symbol->structure = f;
                if (insert(symbol) == -1)
                {
                    semanticError(16, structTag->lineno, "The name of the Struct has been use");
                    free(symbol);
                }
                t->structure.name = structTag->child->name;
            }
            else
                t->structure.name = NULL;
            t->structure.field = f;
        }
    }
    return t;
}

void traverseForDefList(struct TreeNode *p)
{
    assert(strcmp("DefList", p->name) == 0);
    while (p != NULL)
    {
        struct TreeNode *def = p->child;
        struct Type *t = traverseForSpecifier(def->child);

        p = p->child->next;
        if (t == NULL)
            continue;

        struct TreeNode *declist = def->child->next;
        while (declist)
        {
            struct TreeNode *dec = declist->child;
            struct Symbol *s = traverseForVarDec(dec->child, t);
            if (insert(s) == -1)
            {
                semanticError(3, dec->lineno, "The name of Var has been used");
                free(s);
            }
            else
            {
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
    }
}

void traverseForExp(struct TreeNode *p)
{
    struct TreeNode *firstChild = p->child;
    if(strcmp(firstChild->name, "ID") == 0)
    {
        struct Symbol *s = search(firstChild->value.type_str);
        
        if(firstChild->next == NULL)
        {
            p->value.leftFlag = 1;
            if(s == NULL)
            {
                semanticError(1, firstChild->lineno, "Var is not defined");
                p->value.typeForExp = NULL;
                return;
            }
            if(s->flag != S_VAR)
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
                return;
            }
            if(s->flag != S_FUNC)
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
    else if(strcmp(firstChild->name, "LP") == 0)
    {
        traverseForExp(firstChild->next);
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
        if(strcmp(secondChild->name, "DOT") == 0)
        {
            struct Type *t = firstChild->value.typeForExp, *tmp = NULL;
            
            if(t == NULL)
            {
                p->value.typeForExp = NULL;
                return;
            }
            
            struct TreeNode *id = secondChild->next;
            if(t->kind != STRUCTURE)
                semanticError(13, firstChild->lineno, "Var is not a struct var");                   
            else if((tmp = checkStructField(t->structure.field, id->name)) == NULL)
                semanticError(14, id->lineno, "Access the inexisted member of the struct");
            
            p->value.typeForExp = tmp;
        }
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
            if(checkTypeEqual(arrayIndex->value.typeForExp, &_int) == 0)
                semanticError(12, arrayIndex->lineno, "Index of the array should be INT");
            
            p->value.typeForExp = t->array.elem;
        }
        else //binary op(including assignop)
        {
            int flag = strcmp(secondChild->name, "ASSIGNOP");
            struct TreeNode *thirdChild = secondChild->next;
            traverseForExp(thirdChild);

            if(firstChild->value.typeForExp == NULL || thirdChild->value.typeForExp == NULL)
            {
                p->value.typeForExp = NULL;
                return;
            }
            else if(checkTypeEqual(firstChild->value.typeForExp, thirdChild->value.typeForExp) == 0)
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
                if(firstChild->value.typeForExp->kind != BASIC)
                {
                    semanticError(7, firstChild->lineno, "The type doesn't match near operator");
                    p->value.typeForExp = NULL;
                    return;
                }
                if(strcmp(secondChild->name, "OR") == 0 || strcmp(secondChild->name, "AND") == 0)
                {
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

            // else if((flag != 0) && (firstChild->value.typeForExp->kind != BASIC || thirdChild->value.typeForExp->kind != BASIC))
            // {
            //     semanticError(7, firstChild->lineno, "The type doesn't match near operator");
            //     p->value.typeForExp = NULL;
            //     return;
            // }
            
        

        }

    }
    else
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

    // else if(strcmp(firstChild->name, "MINUS") == 0)
    // {
    //     struct TreeNode *exp = firstChild->next;
    //     traverseForExp(exp);
    //     if(checkTypeEqual(exp->value.typeForExp, &_int) == 0 && checkTypeEqual(exp->value.typeForExp, &_float) == 0)
    //     {
    //         semanticError(7, firstChild->lineno, "The operator of MINUS shouble be INT or FLOAT");
    //         p->value.typeForExp = NULL;
    //     }
    //     else
    //         p->value.typeForExp = exp->value.typeForExp;
    // }
    // else if(strcmp(firstChild->name, "NOT") == 0)
    // {
    //     struct TreeNode *exp = firstChild->next;
    //     traverseForExp(exp);
    //     if(checkTypeEqual(exp->value.typeForExp, &_int) == 0)
    //     {
    //         semanticError(7, firstChild->lineno, "The operator of NOT shouble be INT");
    //         p->value.typeForExp = NULL;
    //     }
    //     else
    //         p->value.typeForExp = exp->value.typeForExp;
    // }

    
}

void traverseForStmt(struct TreeNode *p)
{
    struct TreeNode *firstChild = p->child;
    if (strcmp(firstChild->name, "Exp") == 0)
    {
        traverseForExp(firstChild);
    }
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
        if (!(exp->value.typeForExp->kind == BASIC && exp->value.typeForExp->basic == T_INT))
            semanticError(7, exp->lineno, "While condition type should be int");
        traverseForStmt(firstStmt);
    }
    else if (strcmp(firstChild->name, "CompSt") == 0)
        traverseForCompSt(firstChild);
}

void traverseForCompSt(struct TreeNode *p)
{
    assert(strcmp(p->name, "CompSt") == 0);
    struct TreeNode *deflist = p->child->next;
    struct TreeNode *stmtlist = deflist->next;
    if (strcmp("DefList", deflist->name) != 0)
        stmtlist = deflist;
    else
    {
        traverseForDefList(deflist);
    }
    // assert(strcmp(stmtlist->name, "StmtList") == 0);
    if (strcmp(stmtlist->name, "StmtList") == 0)
    {
        while (stmtlist != NULL)
        {
            traverseForStmt(stmtlist->child);
            stmtlist = stmtlist->child->next;
        }
    }
}

void traverseForExtDef(struct TreeNode *p)
{
    assert(strcmp("ExtDef", p->name) == 0);
    struct TreeNode *tmp = p->child->next, *specifier = p->child;
    struct Type *t = traverseForSpecifier(specifier);
    if (strcmp("ExtDecList", tmp->name) == 0)
    {
        if (t == NULL)
            return;
        while (tmp)
        {
            struct TreeNode *vardec = tmp->child;
            struct Symbol *s = traverseForVarDec(vardec, t);
            if (insert(s) == -1)
            {
                semanticError(3, vardec->lineno, "The name of Var has been used");
                free(s);
            }
            if (vardec->next != NULL)
                tmp = vardec->next->next;
            else
                tmp = NULL;
        }
    }
    else if (strcmp("FunDec", tmp->name) == 0)
    {
        struct Symbol *s = (struct Symbol *)malloc(sizeof(struct Symbol));
        s->name = tmp->child->value.type_str;
        s->flag = S_FUNC;
        s->func.returnType = t;

        nowFunc = s;

        struct TreeNode *varlist = tmp->child->next->next;
        if (strcmp(varlist->name, "VarList") == 0)
        {
            s->func.args = traverseForVarList(varlist);
        }
        else
        {
            s->func.argn = 0;
            s->func.args = NULL;
        }

        if (insert(s) == -1)
        {
            semanticError(4, tmp->lineno, "The name of Func has been used");
            free(s);
        }

        traverseForCompSt(tmp->next);
    }
    else
    {
        return;
    }
}

void traverse()
{
    struct TreeNode *extDefList = root->child;
    while (extDefList != NULL)
    {
        traverseForExtDef(extDefList->child);
        extDefList = extDefList->child->next;
    }
}

struct Symbol *search(char *name)
{
    unsigned int hashval = symbolHash(name);
    struct Symbol *s = hashtable[hashval];
    while (s != NULL)
    {
        if (strcmp(name, s->name) == 0)
            return s;
        s = s->hashLinkNext;
    }
    return NULL;
}

int insert(struct Symbol *s)
{
    if (search(s->name) != NULL)
        return -1;
    unsigned int hashval = symbolHash(s->name);
    s->hashLinkNext = hashtable[hashval];
    hashtable[hashval] = s;
    // printf("insert %s\n", hashtable[hashval]->name);
    return 0;
}
void printType(struct Type *t);

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
        struct Symbol *tmp = hashtable[i];
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