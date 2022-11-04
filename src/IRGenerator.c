#include <stdio.h>
#include "tree.h"
#include "semantic.h"
#include "IRGenerator.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
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

struct InterCode* genIfop(struct Operand* result, struct Operand* op1, struct Operand* op2, char *relop)
{
    struct InterCode *p = (struct InterCode *)malloc(sizeof(struct InterCode));
    p->kind = IF;
    p->ifop.result = result;
    p->ifop.op1 = op1;
    p->ifop.op2 = op2;
    
    if(strcmp("==", relop) == 0)
        p->ifop.relop = EQ;
    else if(strcmp("!=", relop) == 0)
        p->ifop.relop = NE;
    else if(strcmp("<", relop) == 0)
        p->ifop.relop = LT;
    else if(strcmp(">", relop) == 0)
        p->ifop.relop = GT;
    else if(strcmp("<=", relop) == 0)
        p->ifop.relop = LE;
    else if(strcmp(">=", relop) == 0)
        p->ifop.relop = GE;
    else 
        assert(0);

    insertIRList(p);
    return p;
}

//return an operand for const int
struct Operand* genConstInt(int value)
{
    struct Operand *p = (struct Operand *)malloc(sizeof(struct Operand));
    p->kind = CONSTANT_INT;
    p->constantValueInt = value;
    return p;
}

//return an operand for const float
struct Operand* genConstFloat(float value)
{
    struct Operand *p = (struct Operand *)malloc(sizeof(struct Operand));
    p->kind = CONSTANT_FLOAT;
    p->constantValueFloat = value;
    return p;
}

//return an operand for a var with kind k, k may be BASICVAR REFVAR ADDRESS LABEL FUNC
struct Operand* genVariable(char *name, enum OperandKind k)
{
    struct Operand *p = (struct Operand *)malloc(sizeof(struct Operand));
    p->kind = k;
    p->name = name;
    return p;
}


char *genTempName()
{
    static int id = 0;
    char *name = (char *)malloc(100);
    strcpy(name, "t");
    sprintf(name + 1, "%d", id);
    ++id;
    return name;
}

char *genVarName(struct TreeNode *p)
{
    struct Symbol * s = search(p->value.type_str);
    assert(s != NULL);
    if(s->IRName != NULL)
        return s->IRName;
    
    static int id = 0;
    char *name = (char *)malloc(100);
    strcpy(name, "v");
    sprintf(name + 1, "%d", id);
    ++id;
    s->IRName = name;
    return name;
}

char *genLabelName()
{
    static int id = 0;
    char *name = (char *)malloc(100);
    strcpy(name, "l");
    sprintf(name + 1, "%d", id);
    ++id;
    return name;
}

//Here Exp must a id or id.id or id[exp]
struct Operand * getLeftOperand(struct TreeNode *exp)
{
    struct TreeNode *firstChild = exp->child, *secondChild = firstChild->next;
    assert(strcmp(firstChild->name, "ID") == 0);
    if(secondChild == NULL)
        return genVariable(genVarName(firstChild), BASCIVAR);
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
    assert(0);
    return 0;
}

void translateArgs(struct TreeNode* args, struct FieldList *formalArgs, int isWriteFunc)
{
    if(isWriteFunc == 1)
    {
        struct TreeNode* exp = args->child;
        struct Operand* tmp = genVariable(genTempName(), BASCIVAR);
        translateExp(exp, tmp);
        genNoresult(WRITE, tmp);
    }
    else
    {
        struct TreeNode* exp = args->child, *comma = exp->next;
        struct TreeNode* child = exp->child;
        struct Operand* tmp = NULL;
        if(formalArgs->type->kind != BASIC)
        {
            //exp must be a struct(no nested struct) or 1-d array
            assert(strcmp(child->name, "ID") == 0);
            tmp = genVariable(genVarName(child), REFVAR);
        }
        else
        {
            tmp = genVariable(genTempName(), BASCIVAR);
            translateExp(exp, tmp);
        }
        if(comma != NULL)
            translateArgs(comma->next, formalArgs->next, 0);
        genNoresult(ARG, tmp);
    }
}
struct FieldList* reverseList(struct FieldList* f)
{
    struct FieldList * ret = copyFieldList(f);
    struct FieldList * before = NULL, *now = ret, *after = ret->next;
    while (after != NULL)
    {
        now->next = before;
        before = now;
        now = after;
        after = after->next;
    }
    now->next = before;
    return now;
    
}
void translateExp(struct TreeNode* exp, struct Operand* place)
{
    assert(strcmp(exp->name, "Exp") == 0);
    struct TreeNode * firstChild = exp->child, *secondChild = firstChild->next;
    // if(place == NULL &&(secondChild == NULL || strcmp(secondChild->name, "ASSIGNOP") != 0))
    //     return;
    
    if(strcmp(firstChild->name, "INT") == 0)
    {
        struct Operand * right = genConstInt(firstChild->value.type_int);
        if(place != NULL)
            genUnaryop(ASSIGN, right, place);
    }
    else if(strcmp(firstChild->name, "FLOAT") == 0)
    {
        struct Operand * right = genConstFloat(firstChild->value.type_float);
        if(place != NULL)
            genUnaryop(ASSIGN, right, place);
    }
    else if(strcmp(firstChild->name, "LP") == 0)
        translateExp(secondChild, place);
    else if(strcmp(firstChild->name, "ID") == 0)
    {
        if(secondChild == NULL)
        {
            //var
            struct Operand * right = genVariable(genVarName(firstChild), BASCIVAR);
            if(place != NULL)
                genUnaryop(ASSIGN, right, place);
        }
        else
        {
            //TODO(): func call
            struct Symbol * func = search(firstChild->value.type_str);
            struct TreeNode* args = firstChild->next->next;
            struct Operand* callfunc = genVariable(func->name, FUNC);
            assert(func != NULL);
            if(strcmp(args->name, "Args") == 0)
            {
                if(strcmp(func->name, "write") == 0)
                {   
                    translateArgs(args, NULL, 1);
                    if(place != NULL)
                        genUnaryop(ASSIGN, genConstInt(0), place);
                    free(callfunc);
                }
                else
                {
                    translateArgs(args, reverseList(func->func.args), 0);
                    if(place != NULL)
                        genUnaryop(FUNCALL, callfunc, place);
                }
            }
            else
            {
                if(strcmp(func->name, "read") == 0)
                {
                    assert(place != NULL);
                    genNoresult(READ, place);
                    free(callfunc);
                }
                else
                {
                    if(place != NULL)
                        genUnaryop(FUNCALL, callfunc, place);
                }
            }
           
        }
    }
    else if(strcmp(secondChild->name, "ASSIGNOP") == 0)
    {
        struct Operand* var = getLeftOperand(firstChild);
        translateExp(secondChild->next, var);
        if(place != NULL)
            genUnaryop(ASSIGN, var, place);
    }
    else if(strcmp(secondChild->name, "RELOP") == 0 || strcmp(secondChild->name, "OR") == 0 || strcmp(secondChild->name, "AND") == 0 || strcmp(firstChild->name, "NOT") == 0)
    {
        //TODO(): Condition Exp
        struct Operand *trueLabel = genVariable(genLabelName(), LABEL), *falseLabel = genVariable(genLabelName(), LABEL);
        struct Operand *falseVal = genConstInt(0), *trueVal = genConstInt(1);
        if(place != NULL)
            genUnaryop(ASSIGN, falseVal, place);
        translateCond(exp, trueLabel, falseLabel);
        genNoresult(LABELDEF, trueLabel);
        if(place != NULL)
            genUnaryop(ASSIGN, trueVal, place);
        genNoresult(LABELDEF, falseLabel);
    }
    else if(strcmp(secondChild->name, "DOT") == 0)
    {
        //TODO(): Struct Field Access
        ;
    }
    else if(strcmp(firstChild->name, "MINUS") == 0)
    {
        struct Operand *tmp1 = genConstInt(0), *tmp2 = genVariable(genTempName(), BASCIVAR);
        translateExp(secondChild, tmp2);
        if(place != NULL)
            genBinaryop(MINUS, place, tmp1, tmp2);
    }
    else
    {
        //arithmetical binary exp
        struct Operand *tmp1 = genVariable(genTempName(), BASCIVAR), *tmp2 = genVariable(genTempName(), BASCIVAR);
        translateExp(firstChild, tmp1);
        translateExp(secondChild->next, tmp2);
        if(place != NULL){
            if(strcmp(secondChild->name, "PLUS") == 0)
                genBinaryop(PLUS, place, tmp1, tmp2);
            else if(strcmp(secondChild->name, "MINUS") == 0)
                genBinaryop(MINUS, place, tmp1, tmp2);
            else if(strcmp(secondChild->name, "STAR") == 0)
                genBinaryop(STAR, place, tmp1, tmp2);
            else if(strcmp(secondChild->name, "DIV") == 0)
                genBinaryop(DIV, place, tmp1, tmp2);
            else
                assert(0);
        }
        
    }
}

void translateCond(struct TreeNode* exp, struct Operand* labelTrue, struct Operand *labelFalse)
{
    assert(strcmp("Exp", exp->name) == 0);
    struct TreeNode *firstChild = exp->child, *secondChild = firstChild->next;
    if(strcmp(secondChild->name, "RELOP") == 0)
    {
        struct Operand *tmp1 = genVariable(genTempName(), BASCIVAR), *tmp2 = genVariable(genTempName(), BASCIVAR);
        translateExp(firstChild, tmp1);
        translateExp(secondChild->next, tmp2);
        genIfop(labelTrue, tmp1, tmp2, secondChild->value.type_str);
        genNoresult(GOTO, labelFalse);
    }
    else if(strcmp(firstChild->name, "NOT") == 0)
        translateCond(secondChild, labelFalse, labelTrue);
    else if(strcmp(secondChild->name, "AND") == 0)
    {
        struct Operand *label = genVariable(genLabelName(), LABEL);
        translateCond(firstChild, label, labelFalse);
        genNoresult(LABELDEF, label);
        translateCond(secondChild->next, labelTrue, labelFalse);
    }
    else if(strcmp(secondChild->name, "OR") == 0)
    {
        struct Operand *label = genVariable(genLabelName(), LABEL);
        translateCond(firstChild, labelTrue, label);
        genNoresult(LABELDEF, label);
        translateCond(secondChild->next, labelTrue, labelFalse);
    }
    else
    {
        struct Operand *tmp = genVariable(genTempName(), BASCIVAR);
        translateExp(exp, tmp);
        genIfop(labelTrue, tmp, genConstInt(0), "!=");
        genNoresult(GOTO, labelFalse);
    }
}

void translateFunDec(struct TreeNode *fundec)
{
    assert(strcmp("FunDec", fundec->name) == 0);
    struct TreeNode* firstChild = fundec->child, *varlist = firstChild->next->next;
    genNoresult(FUNCDEF, genVariable(firstChild->value.type_str, FUNC));
    
    if(strcmp(varlist->name, "VarList") != 0)
        return;

    while(varlist != NULL)
    {
        struct TreeNode * param = varlist->child, *vardec = param->child->next;
        if(param->next == NULL)
            varlist = NULL;
        else
            varlist = param->next->next;
        
        struct TreeNode * id = vardec->child;
        while(strcmp(id->name, "ID") != 0)
            id = id->child;
        
        struct Symbol *s  = search(id->value.type_str);
        assert(s != NULL);
        enum OperandKind k = s->var->kind == BASIC ? BASCIVAR : ADDRESS;        
        genNoresult(PARAM, genVariable(genVarName(id), k));
    }
}

void translateDef(struct TreeNode* def)
{
    assert(strcmp("Def", def->name) == 0);
    struct TreeNode* decList = def->child->next;
    while (decList != NULL)
    {
        struct TreeNode* dec = decList->child;
        if(dec->next != NULL)
            decList = dec->next->next;
        else
            decList = NULL;
        
        struct TreeNode* vardec = dec->child;
        assert(strcmp("VarDec", vardec->name) == 0);
        struct TreeNode* id = vardec->child;
        struct Symbol *s = search(id->value.type_str);
        if(vardec->next == NULL)
        {
            if(s->var->kind == STRUCTURE)
            {
                ;
            }
            else if(s->var->kind == ARRAY)
            {
                ;
            }
            continue;
        }
        while (strcmp(id->name, "ID") != 0)
            id = id->child;
        
        struct Operand *tmp = genVariable(genTempName(), BASCIVAR);
        translateExp(vardec->next->next, tmp);
        genUnaryop(ASSIGN, tmp, genVariable(genVarName(id), BASCIVAR));
    }
    
}

void translateCompst(struct TreeNode *compst)
{
    struct TreeNode* defList = compst->child->next, *stmtList = defList->next;
    if(strcmp("DefList", defList->name) != 0)
        stmtList = defList;
    else
    {
        while (defList != NULL)
        {
            struct TreeNode *def = defList->child;
            defList = def->next;
            translateDef(def);
        }
    }

    if(strcmp("StmtList", stmtList->name) == 0)
    {
        while (stmtList != NULL)
        {
            struct TreeNode* stmt = stmtList->child;
            stmtList = stmt->next;
            translateStmt(stmt);
        }
    }
}
void translateStmt(struct TreeNode *stmt)
{
    assert(strcmp("Stmt", stmt->name) == 0);
    struct TreeNode *firstChild = stmt->child, *secondChild = firstChild->next;
    if(strcmp("Exp", firstChild->name) == 0)
        translateExp(firstChild, NULL);
    else if(strcmp("RETURN", firstChild->name) == 0)
    {
        struct Operand* tmp = genVariable(genTempName(), BASCIVAR);
        translateExp(secondChild, tmp);
        genNoresult(RETURN, tmp);
    }
    else if(strcmp("IF", firstChild->name) == 0)
    {
        struct TreeNode* exp = secondChild->next, *ifStmt = exp->next->next, *elseNode = ifStmt->next;
        if(elseNode == NULL)
        {
            struct Operand *trueLabel = genVariable(genLabelName(), LABEL), *falseLabel = genVariable(genLabelName(), LABEL);
            translateCond(exp, trueLabel, falseLabel);
            genNoresult(LABELDEF, trueLabel);
            translateStmt(ifStmt);
            genNoresult(LABELDEF, falseLabel);
        }
        else
        {
            struct Operand *trueLabel = genVariable(genLabelName(), LABEL), *falseLabel = genVariable(genLabelName(), LABEL), *endlabel = genVariable(genLabelName(), LABEL);
            translateCond(exp, trueLabel, falseLabel);
            genNoresult(LABELDEF, trueLabel);
            translateStmt(ifStmt);
            genNoresult(GOTO, endlabel);
            genNoresult(LABELDEF, falseLabel);
            translateStmt(elseNode->next);
            genNoresult(LABELDEF, endlabel);
        }
    }
    else if(strcmp("WHILE", firstChild->name) == 0)
    {
        struct TreeNode *exp = secondChild->next, *whileStmt = exp->next->next;
        struct Operand *beginLabel = genVariable(genLabelName(), LABEL), *trueLabel = genVariable(genLabelName(), LABEL), *falseLabel = genVariable(genLabelName(), LABEL);
        genNoresult(LABELDEF, beginLabel);
        translateCond(exp, trueLabel, falseLabel);
        genNoresult(LABELDEF, trueLabel);
        translateStmt(whileStmt);
        genNoresult(GOTO, beginLabel);
        genNoresult(LABELDEF, falseLabel);
    }
    else 
        translateCompst(firstChild);
}
void printOperandName(struct Operand *op, FILE * fp)
{
    switch (op->kind)
    {
        case CONSTANT_FLOAT:
            fprintf(fp, "#%f", op->constantValueFloat);
            break;
        case CONSTANT_INT:
            fprintf(fp, "#%d", op->constantValueInt);
            break;
        default:
            fprintf(fp, "%s", op->name);
            break;
    }
}

#define PRINTBINARYOP(op)\
{\
    if(tmp->binaryop.result->kind == BASCIVAR)\
    {\
        assert(tmp->binaryop.op1->kind == BASCIVAR || tmp->binaryop.op1->kind == CONSTANT_INT || tmp->binaryop.op1->kind == CONSTANT_FLOAT);\
        assert(tmp->binaryop.op2->kind == BASCIVAR || tmp->binaryop.op2->kind == CONSTANT_INT || tmp->binaryop.op2->kind == CONSTANT_FLOAT);\
    }\
    else if(tmp->binaryop.result->kind == ADDRESS)\
    {\
        assert(tmp->binaryop.op1->kind == ADDRESS || tmp->binaryop.op1->kind == CONSTANT_INT);\
        assert(tmp->binaryop.op2->kind == ADDRESS || tmp->binaryop.op2->kind == CONSTANT_INT);\
    }\
    else\
        assert(0);\
    fprintf(fp, "%s := ", tmp->binaryop.result->name);\
    printOperandName(tmp->binaryop.op1, fp);\
    fprintf(fp, op);\
    printOperandName(tmp->binaryop.op2, fp);\
    fprintf(fp, "\n");\
}

void printIR(FILE *fp)
{
    struct InterCode *tmp = irList.head->next;
    while (tmp != irList.head)
    {
        switch(tmp->kind)
        {
            case LABELDEF:
                assert(tmp->noresult.op->kind == LABEL);
                fprintf(fp, "LABEL %s :\n", tmp->noresult.op->name);
                break;
            case FUNCDEF:
                assert(tmp->noresult.op->kind == FUNC);
                fprintf(fp, "FUNCTION %s :\n", tmp->noresult.op->name);
                break;
            case ASSIGN:
                assert((tmp->unaryop.left->kind == BASCIVAR && (tmp->unaryop.right->kind == BASCIVAR || tmp->unaryop.right->kind == CONSTANT_FLOAT || tmp->unaryop.right->kind == CONSTANT_INT)) || (tmp->unaryop.left->kind == ADDRESS && (tmp->unaryop.right->kind == ADDRESS || tmp->unaryop.right->kind == CONSTANT_INT)));
                fprintf(fp, "%s := ", tmp->unaryop.left->name);
                printOperandName(tmp->unaryop.right, fp);
                fprintf(fp, "\n");
                break;
            case PLUS:
                PRINTBINARYOP(" + ")
                break;
            case MINUS:
                PRINTBINARYOP(" - ")
                break;
            case STAR:
                PRINTBINARYOP(" * ")
                break;
            case DIV:
                PRINTBINARYOP(" / ")
                break;
            case REF:
                assert(tmp->unaryop.left->kind == ADDRESS && tmp->unaryop.right->kind == REFVAR);
                fprintf(fp, "%s := &%s\n", tmp->unaryop.left->name, tmp->unaryop.right->name);
                break;
            case R_DEREF:
                assert(tmp->unaryop.left->kind == BASCIVAR && tmp->unaryop.right->kind == ADDRESS);
                fprintf(fp, "%s := *%s\n", tmp->unaryop.left->name, tmp->unaryop.right->name);
                break;
            case L_DEREF:
                assert(tmp->unaryop.left->kind == ADDRESS && (tmp->unaryop.right->kind == BASCIVAR || tmp->unaryop.right->kind == CONSTANT_FLOAT || tmp->unaryop.right->kind == CONSTANT_INT));
                fprintf(fp, "*%s := ", tmp->unaryop.left->name);
                printOperandName(tmp->unaryop.right, fp);
                fprintf(fp, "\n");
                break;
            case GOTO:
                assert(tmp->noresult.op->kind == LABEL);
                fprintf(fp, "GOTO %s\n", tmp->noresult.op->name);
                break;
            case IF:
                assert(tmp->ifop.result->kind == LABEL);
                assert(tmp->ifop.op1->kind == BASCIVAR || tmp->ifop.op1->kind == CONSTANT_FLOAT || tmp->ifop.op1->kind == CONSTANT_INT);
                assert(tmp->ifop.op2->kind == BASCIVAR || tmp->ifop.op2->kind == CONSTANT_FLOAT || tmp->ifop.op2->kind == CONSTANT_INT);
                char relop[6][4] = {"==", "!=", "<", ">", "<=", ">="};
                fprintf(fp, "IF ");
                printOperandName(tmp->ifop.op1, fp);
                fprintf(fp, " %s ", relop[tmp->ifop.relop]);
                printOperandName(tmp->ifop.op2, fp);
                fprintf(fp, " GOTO %s\n", tmp->ifop.result->name);
                break;
            case RETURN:
                assert(tmp->noresult.op->kind == BASCIVAR);
                fprintf(fp, "RETURN %s\n", tmp->noresult.op->name);
                break;
            case MALLOC:
                assert(tmp->unaryop.left->kind == BASCIVAR && tmp->unaryop.right->kind == CONSTANT_INT);
                fprintf(fp, "DEC %s %d\n", tmp->unaryop.left->name, tmp->unaryop.right->constantValueInt);
                break;
            case ARG:
                if(tmp->noresult.op->kind == REFVAR)
                    fprintf(fp, "AGR &%s\n", tmp->noresult.op->name);
                else
                {
                    fprintf(fp, "ARG ");
                    printOperandName(tmp->noresult.op, fp);
                    fprintf(fp, "\n");
                }
                break;
            case FUNCALL:
                assert(tmp->unaryop.left->kind == BASCIVAR && tmp->unaryop.right->kind == FUNC);
                fprintf(fp, "%s := CALL %s\n", tmp->unaryop.left->name, tmp->unaryop.right->name);
                break;
            case PARAM:
                assert(tmp->noresult.op->kind == BASCIVAR || tmp->noresult.op->kind == ADDRESS);
                fprintf(fp, "PARAM %s\n", tmp->noresult.op->name);
                break;
            case READ:
                assert(tmp->noresult.op->kind == BASCIVAR);
                fprintf(fp, "READ %s\n", tmp->noresult.op->name);
                break;
            case WRITE:
                assert(tmp->noresult.op->kind == BASCIVAR || tmp->noresult.op->kind == CONSTANT_FLOAT || tmp->noresult.op->kind == CONSTANT_FLOAT);
                fprintf(fp, "WRITE ");
                printOperandName(tmp->noresult.op, fp);
                fprintf(fp, "\n");
                break;
            default:
                assert(0);
        }
        tmp = tmp->next;
    }
    
}