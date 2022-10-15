#include "semantic.h"
#include "tree.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#define HASHSIZE 0x3fff

struct Symbol *hashtable[HASHSIZE] = {};
struct SymbolStack s;

unsigned int symbolHash(char *name)
{
    unsigned int val = 0, i;
    for(; *name; ++name)
    {
        val = (val << 2) + *name;
        if(i = val & (~ HASHSIZE))
            val = (val ^ (i >> 22)) & HASHSIZE;
    }
    return val;
}




struct Symbol * traverseForVarDec(struct TreeNode * p, struct Type *t){
    assert(strcmp("VarDec", p->name));
    struct TreeNode * vardec = p;
    struct Type * tmp1 = t, *tmp2 = t;
    while (strcmp(vardec->child->name, "VarDec") == 0)
    {
        tmp1 = (struct Type *)malloc(sizeof(struct Type));
        tmp1->kind = ARRAY;
        tmp1->array.elem = tmp2;
        tmp1->array.size = vardec->child->next->next->value.type_int;
        tmp2 = tmp1;
        vardec = vardec->child;
    }
    assert(strcmp(vardec->name, "ID") == 0);
    struct Symbol *s = (struct Symbol *)malloc(sizeof(struct Symbol));
    s->flag = S_VAR;
    s->name = vardec->value.type_str;
    s->var = tmp1;
    return tmp1;
}

struct FieldList * traverseForVarList(struct TreeNode *p)
{
    assert(strcmp(p->name, "VarList") == 0);
    struct FieldList * tmp1, *tmp2 = NULL;
    while(p)
    {
        struct TreeNode *paramDec = p->child;
        struct Type* t = traverseForSpecifier(paramDec->child);
        struct Symbol *s =traverseForVarDec(paramDec->child->next, t);
        tmp1 = (struct FieldList *)malloc(sizeof(struct FieldList));
        tmp1->next = tmp2;
        tmp1->name = s->name;
        tmp1->type = t;

        insert(s);
        
        tmp2 = tmp1;

        if(paramDec->next != NULL)
            p = paramDec->next->next;
        else
            p = NULL;
    }
    return tmp2;
}
struct FieldList * formFieldlist(struct TreeNode * p) //p is a deflist
{
    assert(strcmp("DefList", p->name) == 0);
    struct FieldList *tmp1, *tmp2 = NULL;
    while(p != NULL)
    {
        struct TreeNode * def = p->child;
        struct Type * t = traverseForSpecifier(def->child);
        struct TreeNode * declist = def->child->next;

        while(declist)
        {
            struct TreeNode * dec = declist->child;
            struct Symbol * s = traverseForVarDec(dec->child, t);
            insert(s);
            if(dec->child->next!= NULL)
            {
                ;//error
                //initial var in struct is invalid;
            }
            tmp1 = (struct FieldList *)malloc(sizeof(struct FieldList));
            tmp1->name = s->name;
            tmp1->type = s->var;
            tmp1->next = tmp2;
            tmp2 = tmp1;
            if(dec->next != NULL)
                declist = dec->next->next;
            else
                declist = NULL;
        }

        p = p->child->next;
    }
    return tmp2;
}

struct Type * traverseForSpecifier(struct TreeNode* p)
{
    struct Type * t = (struct Type *)malloc(sizeof(struct Type));
    if(strcmp(p->child->name, "TYPE") == 0)
    {
        t->kind = BASIC;
        if(strcmp("int", p->child->value.type_str) == 0)
            t->basic = T_INT;
        else
            t->basic = T_FLOAT;    
    }
    else{
        struct TreeNode * s = p->child;  //StructSpecifier
        struct TreeNode * structTag = s->child->next;
        t->kind = STRUCTURE;
        if(strcmp(structTag->name, "Tag")) //just def a struct val'
        {
            //look up symbol table if the struct has been defined;
            struct TreeNode * structName = structTag->child;
            struct Symbol * symbol = search(structName->value.type_str);
            if(symbol == NULL || symbol->flag != S_STRUCT) 
            {
                //error
                return NULL;
            }
            t->structure.field = symbol->structure;
            t->structure.name = symbol->name;
        }
        else
        {
            int flag = strcmp(structTag->name, "OptTag");
            struct TreeNode * deflist = flag == 0? structTag->next->next : structTag->next;
            struct FieldList * f = formFieldlist(deflist);
            if(flag)
            {
                struct Symbol * symbol = (struct Symbol *)malloc(sizeof(struct Symbol));
                symbol->flag = S_STRUCT;
                symbol->name = structTag->child->name;
                symbol->structure = f;
                insert(s);
                t->structure.name = structTag->child->name;
            }
            else
                t->structure.name = NULL;
            t->structure.field = f;
        }
    }
    return t;
}


void traverseForExtDef(struct TreeNode * p)
{
    assert(strcmp("ExtDef", p->name) == 0);
    struct TreeNode *tmp = p->child->child, *specifier = p->child;
    struct Type *t = traverseForSpecifier(specifier);
    if(strcmp("ExtDecList", tmp->name) == 0)
    {
        while(tmp)
        {
            struct TreeNode *vardec = tmp->child;
            struct Symbol *s = traverseForVarDec(vardec, t);
            insert(s);
            if(vardec->next != NULL)
                tmp = vardec->next->next;
            else
                tmp = NULL;
        }
    }
    else if(strcmp("FunDec", tmp->name) == 0)
    {
        struct Symbol * s = (struct Symbol *)malloc(sizeof(struct Symbol));
        s->name = tmp->child->name;
        s->flag = S_FUNC;
        s->func.returnType = t;
        struct TreeNode *varlist = tmp->child->next->next;
        if(strcmp(varlist, "VarList"))
        {
            s->func.args = traverseForVarList(varlist);
        }
        else
        {
            s->func.argn = 0;
            s->func.args = NULL;
        }


        insert(s);
    }
    else
    {
        return;
    }
}


void traverse(struct TreeNode* p)
{
    if(strcmp(p->name, "ExtDef") == 0)
    {
        
        return;
    }
    while(p->child != NULL)
    {
        traverse(p->child);
    }
}


struct Symbol * search(char *name)
{
    return NULL;
}

void insert(struct Symbol *s)
{

}