#include "IRGenerator.h"
#include "tree.h"
#include "semantic.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
void initIRGenerator()
{
    irList.head = (struct InterCode*)malloc(sizeof(struct InterCode));
    irList.head->next = irList.head;
    irList.head->prev = irList.head;
    irList.tail = irList.head;
}

void insertIRList(struct InterCode * p)
{
    irList.tail->next = p;
    p->prev = irList.tail;
    irList.head->prev = p;
    p->next = irList.head;
    irList.tail = p;
}

struct InterCode* genUnaryop(enum InterCodeKind k, struct Operand* right, struct Operand* left)
{
    struct InterCode *p = (struct InterCode *)malloc(sizeof(struct InterCode));
    p->kind = k;
    p->unaryop.left = left;
    p->unaryop.right = right;
    insertIRList(p);
    return p;
}

struct InterCode* genBinaryop(enum InterCodeKind k, struct Operand* result, struct Operand* op1, struct Operand* op2)
{
    struct InterCode *p = (struct InterCode *)malloc(sizeof(struct InterCode));
    p->kind = k;
    p->binaryop.result = result;
    p->binaryop.op1 = op1;
    p->binaryop.op2 = op2;
    insertIRList(p);
    return p;
}

struct InterCode* genNoresult(enum InterCodeKind k, struct Operand* op)
{
    struct InterCode *p = (struct InterCode *)malloc(sizeof(struct InterCode));
    p->kind = k;
    p->noresult.op = op;
    insertIRList(p);
    return p;
}

struct Operand* genConstInt(int value)
{
    struct Operand *p = (struct Operand *)malloc(sizeof(struct Operand));
    p->kind = CONSTANT_INT;
    p->constantValueInt = value;
    return p;
}

struct Operand* genConstFloat(float value)
{
    struct Operand *p = (struct Operand *)malloc(sizeof(struct Operand));
    p->kind = CONSTANT_FLOAT;
    p->constantValueFloat = value;
    return p;
}

struct Operand* genBasicVariable(char *name)
{
    struct Operand *p = (struct Operand *)malloc(sizeof(struct Operand));
    p->kind = BASCIVAR;
    p->varName = name;
    return p;
}

char *genTempName()
{
    static int id = 0;
    char *name = (char *)malloc(100);
    strcpy(name, "temppp");
    sprintf(name + 6, "%d", id);
    ++id;
    return name;
}

//Here Exp must a id or id.id or id[exp]
struct Operand * getLeftOperand(struct TreeNode *exp)
{
    struct TreeNode *firstChild = exp->child, *secondChild = firstChild->next;
    if(secondChild == NULL)
        return genBasicVariable(firstChild->value.type_str);
    //TODO()
    //array and struct

    
    return NULL;
}

enum InterCodeKind determineKind(struct Operand* right, struct Operand* left)
{
    if(left->kind == BASCIVAR)
    {
        if(right->kind == CONSTANT_FLOAT || right->kind == CONSTANT_INT || right->kind == BASCIVAR)
            return ASSIGN;
        if(right->kind == ADDRESS)
            return R_DEREF;
    }
    else if(left->kind == ADDRESS)
    {
        assert(right->kind == BASCIVAR && right->kind == CONSTANT_FLOAT && right->kind == CONSTANT_INT);
        return L_DEREF;
    }
}

void translateExp(struct TreeNode* exp, struct Operand* place)
{
    assert(strcmp(exp->name, "Exp") == 0);
    struct TreeNode * firstChild = exp->child, *secondChild = firstChild->next;
    if(place == NULL &&(secondChild == NULL || strcmp(secondChild->name, "ASSIGNOP") != 0))
        return;
    
    if(strcmp(firstChild->name, "INT") == 0)
    {
        struct Operand * right = genConstInt(firstChild->value.type_int);
        genUnaryop(determineKind(right, place), right, place);
    }
    else if(strcmp(firstChild->name, "FLOAT") == 0)
    {
        struct Operand * right = genConstFloat(firstChild->value.type_float);
        genUnaryop(determineKind(right, place), right, place);
    }
    else if(strcmp(firstChild->name, "LP") == 0)
        translateExp(secondChild, place);
    else if(strcmp(firstChild->name, "ID") == 0)
    {
        if(secondChild == NULL)
        {
            //var
            struct Operand * right = genBasicVariable(firstChild->value.type_str);
            genUnaryop(determineKind(right, place), right, place);
        }
        else
        {
            //TODO(): func call
            ;
        }
    }
    else if(strcmp(secondChild->name, "ASSIGNOP") == 0)
    {
        struct Operand* var = getLeftOperand(firstChild);
        translateExp(secondChild->next, var);
        if(place != NULL)
            genUnaryop(determineKind(var, place), var, place);
    }
    else if(strcmp(secondChild->name, "RELOP") == 0 || strcmp(secondChild->name, "OR") == 0 || strcmp(secondChild->name, "AND") == 0 || strcmp(firstChild->name, "NOT") == 0)
    {
        //TODO(): Condition Exp
        ;
    }
    else if(strcmp(secondChild->name, "DOT") == 0)
    {
        //TODO(): Struct Field Access
        ;
    }
    else
    {
        //arithmetical binary exp
        struct Operand *tmp1 = genBasicVariable(genTempName()), *tmp2 = genBasicVariable(genTempName());
        translateExp(firstChild, tmp1);
        translateExp(secondChild->next, tmp2);
        if(strcmp(secondChild->name, "ADD") == 0)
            genBinaryop(PLUS, place, tmp1, tmp2);
        else if(strcmp(secondChild->name, "MINUS") == 0)
            genBinaryop(MINUS, place, tmp1, tmp2);
        else if(strcmp(secondChild->name, "STAR") == 0)
            genBinaryop(STAR, place, tmp1, tmp2);
        else if(strcmp(secondChild->name, "DIV"));
            genBinaryop(DIV, place, tmp1, tmp2);
        else
            assert(0);
    }
}

void translateCond(struct TreeeNode* exp, struct Operand* labelTrue, struct Operand *labelFalse)
{
    ;
}
