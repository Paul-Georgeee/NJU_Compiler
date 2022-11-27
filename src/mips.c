#include <stdio.h>
#include "tree.h"
#include "semantic.h"
#include "IRGenerator.h"
#include "controlFlowGraph.h"
#include "mips.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define zero    0
#define at      1
#define v0      2
#define v1      3
#define a0      4
#define a1      5
#define a2      6
#define a3      7
#define t0      8
#define t1      9
#define t2      10
#define t3      11
#define t4      12
#define t5      13
#define t6      14
#define t7      15
#define s0      16
#define s1      17
#define s2      18
#define s3      19
#define s4      20
#define s5      21
#define s6      22
#define s7      23
#define t8      24
#define t9      25
#define k0      26
#define k1      27
#define gp      28
#define sp      29
#define fp      30
#define ra      31

char *regName[] = { "$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3",
                    "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7",
                    "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
                    "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra"
                };
int usedForVar[] = {t0, t1, t2, t3, t4, t5, t6, t7, s0, s1, s2, s3, s4, s5, s6, s7, t8, t9};
#define VarRegSize (sizeof(usedForVar) / sizeof(int))
//对于regs数组中的contain，只会在allocateReg中和用于暂时存在中间代码中的常数后修改
//对于regs数组中的op，只会在allocateReg中和regForDefvar中修改
//对于每个Operand，其中的regIndex仅在allocateReg中当被替换时修改为NOTINREG，在ensureReg和regForDefVar中修改分配到的寄存器
//其中的offsetByfp只会在allocateReg函数中被替换时修改
int allocateReg(FILE *f, struct InterCode* nowCode)
{
    //first find if there is a reg is free
    for(int i = 0; i < VarRegSize; ++i)
    {
        if(regs[usedForVar[i]].contain == 0)
        {
            regs[usedForVar[i]].contain = 1;
            regs[usedForVar[i]].op = NULL;
            return usedForVar[i];
        }
    }

    //choose a reg that is the farest one to be used
    //记得找到对应的寄存器后，将寄存器原来包含的变量op的regIndex改为NOINREG，
    //并写回内存

    int index = -1, nextUsed = 0;
    for(int i = 0; i < VarRegSize; ++i)
    {
        struct Operand *op = regs[usedForVar[i]].op;
        if(op == NULL)
            continue;
        
        struct UseList *use = op->useList.next;
        while(use != NULL && use->code->no < nowCode->no)
        {
            struct UseList *tmp = use;
            use = use->next;
            free(tmp);
        }
        op->useList.next = use;

        if(use == NULL)
        {
            index = i;
            break;
        }
        else
        {
            int diff = use->code->no - nowCode->no;
            if(diff >= nextUsed)
            {
                index = i;
                nextUsed = diff;
            }
        }
    }
    assert(index != -1);
    assert(regs[usedForVar[index]].op->offsetByfp != NOTINMEM);
    // if(regs[usedForVar[index]].op->offsetByfp == NOTINMEM)
    // {
    //     funcFrame.size += 4;
    //     regs[usedForVar[index]].op->offsetByfp = -funcFrame.size;
    //     fprintf(f, "    addi $sp, $sp, -4\n");
    // }
    fprintf(f, "    sw %s, %d($fp)\n", regName[usedForVar[index]], regs[usedForVar[index]].op->offsetByfp);
    regs[usedForVar[index]].op->regIndex = NOTINREG;
    regs[usedForVar[index]].op = NULL;
    return usedForVar[index];
}

void regForDefVar(struct Operand *op, FILE *f, struct InterCode* nowCode)
{
    if(op->regIndex == NOTINREG)
    {
        int regIndex = allocateReg(f, nowCode);
        assert(op->offsetByfp != NOTINMEM);
        fprintf(f, "    lw %s, %d($fp)\n", regName[regIndex], op->offsetByfp);
        op->regIndex = regIndex;
        regs[regIndex].op = op;
    }
}

int ensureReg(struct Operand *op, FILE *f, struct InterCode* nowCode)
{
    if(op->regIndex == NOTINREG)
    {
        assert(op->offsetByfp != NOTINMEM);
        op->regIndex = allocateReg(f, nowCode);
        regs[op->regIndex].op = op;
        fprintf(f, "    lw %s, %d($fp)\n", regName[op->regIndex], op->offsetByfp);
    }
    return op->regIndex;
}


void getUseInfo(struct InterCode* blockBegin, struct InterCode* blockEnd)
{
    struct UseList *uleft, *uright;
    struct UseList *uresult, *uop1, *uop2;
    struct UseList *uop;
    assert(blockBegin != blockEnd);
    struct InterCode* tmp = blockEnd->prev;
    while(tmp != blockBegin)
    {
        switch (tmp->kind)
        {
        case ASSIGN: case REF: case L_DEREF: case R_DEREF:
            uleft = (struct UseList*)malloc(sizeof(struct UseList));
            uright = (struct UseList*)malloc(sizeof(struct UseList));
            memset(uleft, 0, sizeof(struct UseList));
            memset(uright, 0, sizeof(struct UseList));
            uleft->next = tmp->unaryop.left->useList.next;
            tmp->unaryop.left->useList.next = uleft;
            
            uright->next = tmp->unaryop.right->useList.next;
            tmp->unaryop.right->useList.next = uright;
            
            uleft->useOrDef = 0;
            uright->useOrDef = 1;
            uleft->code = uright->code = tmp;
            break;
        case PLUS: case MINUS: case DIV: case STAR:
            uresult = (struct UseList*)malloc(sizeof(struct UseList));
            uop1 = (struct UseList*)malloc(sizeof(struct UseList));
            uop2 = (struct UseList*)malloc(sizeof(struct UseList));            
            memset(uresult, 0, sizeof(struct UseList));
            memset(uop1, 0, sizeof(struct UseList));
            memset(uop2, 0, sizeof(struct UseList));
            uresult->next = tmp->binaryop.result->useList.next;
            tmp->binaryop.result->useList.next = uresult;

            uop1->next = tmp->binaryop.op1->useList.next;
            tmp->binaryop.op1->useList.next = uop1;

            uop2->next = tmp->binaryop.op2->useList.next;
            tmp->binaryop.op2->useList.next = uop2;

            uresult->useOrDef = 0;
            uop1->useOrDef = uop2->useOrDef = 1;
            uresult->code = uop1->code = uop2->code = tmp;
            break;
        case IF:
            uop1 = (struct UseList*)malloc(sizeof(struct UseList));
            uop2 = (struct UseList*)malloc(sizeof(struct UseList));
            memset(uop1, 0, sizeof(struct UseList));
            memset(uop2, 0, sizeof(struct UseList));
            uop1->next = tmp->ifop.op1->useList.next;
            tmp->ifop.op1->useList.next = uop1;

            uop2->next = tmp->ifop.op2->useList.next;
            tmp->ifop.op2->useList.next = uop2;

            uop1->useOrDef = uop2->useOrDef = 1;
            uop1->code = uop2->code = tmp;
            break;
        case ARG: case READ: case WRITE: case RETURN:
            uop = (struct UseList*)malloc(sizeof(struct UseList));
            memset(uop, 0, sizeof(struct UseList));
            uop->next = tmp->noresult.op->useList.next;
            tmp->noresult.op->useList.next = uop;
            break;
        case FUNCALL: case MALLOC:
            uleft = (struct UseList*)malloc(sizeof(struct UseList));
            memset(uleft, 0, sizeof(struct UseList));
            uleft->next = tmp->unaryop.left->useList.next;
            tmp->unaryop.left->useList.next = uleft;
            uleft->useOrDef = 0;
            uleft->code = tmp;
            break;
        default:
            break;
        }
        tmp = tmp->prev;
    }
}

#define FREEUSELIST \
{\
    while (u != NULL)\
    {\
        freeU = u;\
        u = u->next;\
        free(freeU);\
    }\
}
void clearUseInfo(struct InterCode* blockBegin, struct InterCode* blockEnd)
{
    struct InterCode *tmp = blockBegin;
    while (tmp != blockEnd)
    {
        struct UseList *u, *freeU;
        struct Operand *left, *right;
        struct Operand *result, *op1, *op2;
        struct Operand *op;
        switch (tmp->kind)
        {
        case ASSIGN: case REF: case L_DEREF: case R_DEREF: case MALLOC: case FUNCALL:
            left = tmp->unaryop.left;
            right = tmp->unaryop.right;
            u = left->useList.next;
            FREEUSELIST
            left->useList.next = NULL;
            u = right->useList.next;
            FREEUSELIST
            right->useList.next = NULL;
            break;
        case PLUS: case MINUS: case STAR: case DIV:
            result = tmp->binaryop.result;
            op1 = tmp->binaryop.op1;
            op2 = tmp->binaryop.op2;
            u = result->useList.next;
            FREEUSELIST
            result->useList.next = NULL;
            u = op1->useList.next;
            FREEUSELIST
            op1->useList.next = NULL;
            u = op2->useList.next;
            FREEUSELIST
            op2->useList.next = NULL;
            break;
        case LABELDEF: case FUNCDEF: case GOTO: case RETURN: case ARG: case PARAM: case READ: case WRITE:
            op = tmp->noresult.op;
            u = op->useList.next;
            FREEUSELIST
            op->useList.next = NULL;
            break;
        case IF:
            result = tmp->ifop.result;
            op1 = tmp->ifop.op1;
            op2 = tmp->ifop.op2;
            u = result->useList.next;
            FREEUSELIST
            result->useList.next = NULL;
            u = op1->useList.next;
            FREEUSELIST
            op1->useList.next = NULL;
            u = op2->useList.next;
            FREEUSELIST
            op2->useList.next = NULL;
            break;
        default:
            break;
        }
        tmp = tmp->next;
    }
}

int calculateSaveSize()
{
    int stackdiff = 0;
    for(int i = 0; i < VarRegSize; ++i)
    {
        int index = usedForVar[i];
        if(regs[index].contain == 1 && regs[index].op != NULL)
        {
            struct Operand *op = regs[index].op;
            if(op->offsetByfp == NOTINMEM)
            {
                funcFrame.size += 4;
                op->offsetByfp = -funcFrame.size;
                stackdiff += 4;
            }
        }
    }
    return stackdiff;
}
void saveVarToMem(FILE *f)
{
    // int stackdiff = calculateSaveSize();
    // if(stackdiff != 0)
    //     fprintf(f, "    addi $sp, $sp, -%d\n", stackdiff);
    for(int i = 0; i < VarRegSize; ++i)
    {
        int index = usedForVar[i];
        if(regs[index].contain == 1)
        {
            regs[index].contain = 0;
            if(regs[index].op != NULL)
            {
                assert(regs[index].op->offsetByfp != NOTINMEM);
                fprintf(f, "    sw %s, %d($fp)\n", regName[index], regs[index].op->offsetByfp);
                regs[index].op->regIndex = NOTINREG;
                regs[index].op = NULL;
            }
        }

    }
}

static char *relopInstr[] = {"beq", "bne", "blt", "bgt", "ble", "bge"};

void genMipsForStarAndDiv(struct InterCode *tmp, FILE *f)
{
    struct Operand *result, *op1, *op2;
    int op1Reg, op2Reg;
    result = tmp->binaryop.result;
    op1 = tmp->binaryop.op1;
    op2 = tmp->binaryop.op2;
    
    regForDefVar(result, f, tmp);
    if(op1->kind == CONSTANT_INT)
    {
        op1Reg = allocateReg(f, tmp);
        fprintf(f, "    li %s, %d\n", regName[op1Reg], op1->constantValueInt);
    }
    else
        op1Reg = ensureReg(op1, f, tmp);
    
    if(op2->kind == CONSTANT_INT)
    {
        op2Reg = allocateReg(f, tmp);
        fprintf(f, "    li %s, %d\n", regName[op2Reg], op2->constantValueInt);
    }
    else
        op2Reg = ensureReg(op2, f, tmp);
    
    if(tmp->kind == DIV)
    {
        fprintf(f, "    div %s, %s\n", regName[op1Reg], regName[op2Reg]);
        fprintf(f, "    mflo %s\n", regName[result->regIndex]);
    }
    else
        fprintf(f, "    mul %s, %s, %s\n", regName[result->regIndex], regName[op1Reg], regName[op2Reg]);
    
    if(op1->kind == CONSTANT_INT)
        regs[op1Reg].contain = 0;
    if(op2->kind == CONSTANT_INT)
        regs[op2Reg].contain = 0;
}
void genMipsForPlusAndMinus(struct InterCode *tmp, FILE *f)
{
    struct Operand *result, *op1, *op2;
    int op1Reg;
    result = tmp->binaryop.result;
    op1 = tmp->binaryop.op1;
    op2 = tmp->binaryop.op2;
    
    regForDefVar(result, f, tmp);
    if(op1->kind == CONSTANT_INT)
    {
        op1Reg = allocateReg(f, tmp);
        fprintf(f, "    li %s, %d\n", regName[op1Reg], op1->constantValueInt);
    }
    else
        op1Reg = ensureReg(op1, f, tmp);
    
    if(op2->kind == CONSTANT_INT)
    {
        if(tmp->kind == PLUS)
            fprintf(f, "    addi %s, %s, %d\n", regName[result->regIndex], regName[op1Reg], op2->constantValueInt);
        else
            fprintf(f, "    subi %s, %s, %d\n", regName[result->regIndex], regName[op1Reg], op2->constantValueInt);
    }
    else
    {
        if(tmp->kind == PLUS)
            fprintf(f, "    add %s, %s, %s\n", regName[result->regIndex], regName[op1Reg], regName[ensureReg(op2, f, tmp)]);
        else
            fprintf(f, "    sub %s, %s, %s\n", regName[result->regIndex], regName[op1Reg], regName[ensureReg(op2, f, tmp)]);
    }
    
    if(op1->kind == CONSTANT_INT)
        regs[op1Reg].contain = 0;
}
void genMipsForIF(struct InterCode *tmp, FILE *f)
{
    struct Operand *op1, *op2;
    int op1Reg, op2Reg;
    op1 = tmp->ifop.op1;
    op2 = tmp->ifop.op2;
    if(op1->kind == CONSTANT_INT)
    {
        op1Reg = allocateReg(f, tmp);
        fprintf(f, "    li %s, %d\n", regName[op1Reg], op1->constantValueInt);
    }
    else
        op1Reg = ensureReg(op1, f, tmp);
    
    if(op2->kind == CONSTANT_INT)
    {
        op2Reg = allocateReg(f, tmp);
        fprintf(f, "    li %s, %d\n", regName[op2Reg], op2->constantValueInt);
    }
    else
        op2Reg = ensureReg(op2, f, tmp);
    
    fprintf(f, "    %s %s, %s, %s\n", relopInstr[tmp->ifop.relop], regName[op1Reg], regName[op2Reg], tmp->ifop.result->name);
    if(op1->kind == CONSTANT_INT)
        regs[op1Reg].contain = 0;
    if(op2->kind == CONSTANT_INT)
        regs[op2Reg].contain = 0;

}
void genMipsForDeref(struct InterCode *tmp, FILE *f)
{
    struct Operand *left, *right;
    int rightReg;
    left = tmp->unaryop.left;
    right = tmp->unaryop.right;
    regForDefVar(left, f, tmp);
    if(tmp->kind == R_DEREF)
        fprintf(f, "    lw %s, 0(%s)\n", regName[left->regIndex], regName[ensureReg(right, f, tmp)]);
    else
    {
        if(right->kind == CONSTANT_INT)
        {
            rightReg = allocateReg(f, tmp);
            regs[rightReg].contain = 0;
            fprintf(f, "    li %s, %d\n", regName[rightReg], right->constantValueInt);
        }
        else
            rightReg = ensureReg(right, f, tmp);
        fprintf(f, "    sw %s, 0(%s)\n", regName[rightReg], regName[left->regIndex]);
    }
}
void genMipsForMalloc(struct InterCode *tmp, FILE *f)
{
    struct Operand *left, *right;
    left = tmp->unaryop.left;
    right = tmp->unaryop.right;
    assert(right->kind == CONSTANT_INT);
    funcFrame.size += right->constantValueInt;
    fprintf(f, "    addi $sp, $sp, -%d\n", right->constantValueInt);
    
    regForDefVar(left, f, tmp);
    fprintf(f, "    move %s, $sp\n", regName[left->regIndex]);
}

static inline void saveArgReg(FILE *f)
{
    for(int i = a0; i <= a3; ++i)
    {
        struct Operand *op = regs[i].op;
        if(op != NULL)
        {
            assert(op->offsetByfp != NOTINMEM);
            fprintf(f, "    sw %s, %d($fp)\n", regName[i], op->offsetByfp);
        }
    }
}
static inline void recoverArgReg(FILE *f)
{
    for(int i = a0; i <= a3; ++i)
    {
        struct Operand *op = regs[i].op;
        if(op != NULL)
        {
            assert(op->offsetByfp != NOTINMEM);
            fprintf(f, "    lw %s, %d($fp)\n", regName[i], op->offsetByfp);
        }
    }
}
struct InterCode * genMipsForFuncall(struct InterCode *argBegin, FILE *f)
{
    int argcnt = 0;
    struct InterCode *tmp = argBegin;
    //int stackdiff = 0;
    while (tmp->kind == ARG)
    {
        if(tmp->noresult.op->offsetByfp == NOTINMEM)
            assert(0);
            // funcFrame.size += 4;
            // tmp->noresult.op->offsetByfp = -funcFrame.size;
            // stackdiff += 4;
        
        argcnt++;
        tmp = tmp->next;
    }

    assert(tmp->kind == FUNCALL);
    

    if(argcnt > 4)
        fprintf(f, "    addi $sp, $sp, -%d\n", (argcnt - 4) * 4);
    // else if(stackdiff != 0)
    //     fprintf(f, "    addi $sp, $sp, -%d\n", stackdiff);
    struct InterCode *args = argBegin;
    
    saveArgReg(f);
    for(int i = argcnt - 1; i >= 0; --i)
    {
        int regIndex = ensureReg(args->noresult.op, f, args);
        if(i < 4)
            fprintf(f, "    move %s, %s\n", regName[a0 + i], regName[regIndex]);
        else
            fprintf(f, "    sw %s, %d($sp)\n", regName[regIndex], (i - 4) * 4);
        
        args = args->prev;
    }
    saveVarToMem(f);
    
    fprintf(f, "    jal %s\n", tmp->unaryop.right->name);
    if(argcnt > 4)
        fprintf(f, "    addi $sp, $sp, %d\n", (argcnt - 4) * 4);
    regForDefVar(tmp->unaryop.left, f, tmp);
    fprintf(f, "    move %s, $v0\n", regName[tmp->unaryop.left->regIndex]);
    recoverArgReg(f);
    return tmp;
}

void genMipsForRead(struct InterCode *tmp, FILE *f)
{
    assert(tmp->kind == READ);
    int stackdiff = calculateSaveSize();
    if(stackdiff != 0)
        assert(0);
    saveVarToMem(f);
    fprintf(f, "    jal read\n");
    regForDefVar(tmp->noresult.op, f, tmp);
    fprintf(f, "    move %s, $v0\n", regName[tmp->noresult.op->regIndex]);
}

void genMipsForWrite(struct InterCode *tmp, FILE *f)
{
    assert(tmp->kind == WRITE);
    int stackdiff = calculateSaveSize();
    if(stackdiff != 0)
        assert(0);
    fprintf(f, "    move $a0, %s\n", regName[ensureReg(tmp->noresult.op, f, tmp)]);
    saveVarToMem(f);
    fprintf(f, "    jal write\n");
}

void genMipsForReturn(struct InterCode *tmp, FILE *f)
{
    assert(tmp->kind == RETURN);
    if(funcFrame.hasFunCall == 1) //recover the return address
        fprintf(f, "    lw $ra, %d($fp)\n", funcFrame.returnAdd);
    //save return value
    fprintf(f, "    move $v0, %s\n", regName[ensureReg(tmp->noresult.op, f, tmp)]);
      
    //recover the fp register
    fprintf(f, "    lw $fp, %d($sp)\n", funcFrame.size);
    //recover the stack
    fprintf(f, "    addi $sp, $sp, %d\n", funcFrame.size + 4);
    //return
    fprintf(f, "    jr $ra\n"); 
}

struct InterCode * genMipsForSingle(struct InterCode *tmp, FILE *f)
{
    struct Operand *left, *right;
    switch (tmp->kind)
    {
    case ASSIGN: case REF:
        left = tmp->unaryop.left;
        right = tmp->unaryop.right;
        
        regForDefVar(left, f, tmp);
        if(right->kind == CONSTANT_INT)
            fprintf(f, "    li %s, %d\n", regName[left->regIndex], right->constantValueInt);
        else
            fprintf(f, "    move %s, %s\n", regName[left->regIndex], regName[ensureReg(right, f, tmp)]);
        break;
    case PLUS: case MINUS:
        genMipsForPlusAndMinus(tmp, f);
        break;
    case DIV: case STAR:
        genMipsForStarAndDiv(tmp, f);
        break;
    case L_DEREF: case R_DEREF:
        genMipsForDeref(tmp, f);
        break;
    case IF:
        genMipsForIF(tmp, f);
        break;
    case MALLOC:
        genMipsForMalloc(tmp, f);
        break;
    case LABELDEF:
        fprintf(f, "%s:\n", tmp->noresult.op->name);
        break;
    case GOTO:
        fprintf(f, "    j %s\n", tmp->noresult.op->name);
        break;
    case ARG: case FUNCALL:
        tmp = genMipsForFuncall(tmp, f);
        break;
    case READ:
        genMipsForRead(tmp, f);
        break;
    case WRITE:
        genMipsForWrite(tmp, f);
        break;
    case RETURN:
        assert(0);
        break;
    default:
        break;
    }
    return tmp;
}

static inline void allocateMemForVar(struct Operand *op)
{
    if(op->kind == CONSTANT_INT || op->kind == CONSTANT_FLOAT || op->kind == LABEL || op->kind == FUNC)
        return;
    if(op->offsetByfp != NOTINMEM)
        return;
    funcFrame.size += 4;
    op->offsetByfp = -funcFrame.size;
}

void allocateMemForFunc(struct InterCode *funcBegin, struct InterCode *funcEnd)
{
    struct InterCode *tmp = funcBegin;
    while (tmp != funcEnd)
    {

        switch (tmp->kind)
        {
        case PLUS: case MINUS: case STAR: case DIV:
            allocateMemForVar(tmp->binaryop.result);
            allocateMemForVar(tmp->binaryop.op1);
            allocateMemForVar(tmp->binaryop.op2);
            break;
        case IF:
            allocateMemForVar(tmp->ifop.op1);
            allocateMemForVar(tmp->ifop.op2);
            break;
        case ASSIGN: case REF: case L_DEREF: case R_DEREF: case MALLOC: case FUNCALL:
            allocateMemForVar(tmp->unaryop.left);
            allocateMemForVar(tmp->unaryop.right);
            break;
        case RETURN: case READ: case WRITE:
            allocateMemForVar(tmp->noresult.op);
            break;
        default:
            break;
        }
        tmp = tmp->next;
    }
    
}
void genMipsForFunc(struct InterCode* funcBegin, struct InterCode* funcEnd, FILE *f)
{
    assert(funcBegin->kind == FUNCDEF);
    memset(regs, 0, sizeof(regs));
    memset(&funcFrame, 0, sizeof(struct FuncFrame));
    fprintf(f, "%s:\n", funcBegin->noresult.op->name);

    struct InterCode *tmp = funcBegin->next;
    int argCnt = 0;
    while (tmp->kind == PARAM)
    {
        if(argCnt < 4)
        {
            tmp->noresult.op->regIndex = a0 + argCnt;
            tmp->noresult.op->offsetByfp = NOTINMEM;
            regs[a0 + argCnt].contain = 1;
            regs[a0 + argCnt].op = tmp->noresult.op;
        }
        else
        {
            tmp->noresult.op->regIndex = NOTINREG;
            tmp->noresult.op->offsetByfp = (argCnt - 3) * 4;
        }
        argCnt = argCnt + 1;
        tmp = tmp->next;
    }
    //save fp
    fprintf(f, "    addi $sp, $sp, -4\n");
    fprintf(f, "    sw $fp, 0($sp)\n");
    fprintf(f, "    move $fp, $sp\n");
        
    struct InterCode * detectFunCall = tmp;
    while (detectFunCall != funcEnd)
    {
        if(detectFunCall->kind == FUNCALL || detectFunCall->kind == READ || detectFunCall->kind == WRITE)
            funcFrame.hasFunCall = 1;
        detectFunCall = detectFunCall->next;
    }
    if(funcFrame.hasFunCall == 1)
    {
        funcFrame.size += 4;
        funcFrame.returnAdd = -funcFrame.size;
        fprintf(f, "    addi $sp, $sp, -4\n");
        fprintf(f, "    sw $ra, 0($sp)\n");
    }
    
    int oldsize = funcFrame.size;
    allocateMemForFunc(tmp, funcEnd);
    if(funcFrame.size > 0)
        fprintf(f, "    addi $sp, $sp, %d\n", -(funcFrame.size - oldsize));

    constructCFG(tmp, funcEnd);
    for(int i = 1; i < cfg.nodeCnt - 1; ++i)
    {
        struct InterCode * blockBegin = cfg.nodes[i].begin, *blockEnd = cfg.nodes[i].end;
        getUseInfo(blockBegin, blockEnd);
        struct InterCode * tmp = blockBegin;
        while (tmp != blockEnd->prev)
        {    
            tmp = genMipsForSingle(tmp, f);
            tmp = tmp->next;
        }
        //specially handle for the last inter code in the basic block
        if(tmp->kind == GOTO)
        {
            saveVarToMem(f);
            genMipsForSingle(tmp, f);
        }
        else if(tmp->kind == RETURN)
        {
            genMipsForReturn(tmp, f);
        }
        else if(tmp->kind == IF)
        {
            struct Operand *op1, *op2;
            int op1Reg, op2Reg;
            op1 = tmp->ifop.op1;
            op2 = tmp->ifop.op2;
            if(op1->kind == CONSTANT_INT)
            {
                op1Reg = allocateReg(f, tmp);
                fprintf(f, "    li %s, %d\n", regName[op1Reg], op1->constantValueInt);
            }
            else
                op1Reg = ensureReg(op1, f, tmp);
            
            if(op2->kind == CONSTANT_INT)
            {
                op2Reg = allocateReg(f, tmp);
                fprintf(f, "li %s, %d\n", regName[op2Reg], op2->constantValueInt);
            }
            else
                op2Reg = ensureReg(op2, f, tmp);
            saveVarToMem(f);
            fprintf(f, "    %s %s, %s, %s\n", relopInstr[tmp->ifop.relop], regName[op1Reg], regName[op2Reg], tmp->ifop.result->name);
        }
        else
        {
            genMipsForSingle(tmp, f);
            saveVarToMem(f);
        }

        clearUseInfo(blockBegin, blockEnd);
        
        
    }
}

void genMips(struct InterCode* begin, struct InterCode* end, FILE *f)
{
    struct InterCode* tmp = begin;
    struct InterCode* funcBegin = begin;
    char *s = ".data\n"
    "_prompt: .asciiz \"Enter an integer:\"\n"
    "_ret: .asciiz \"\\n\"\n"
    ".globl main\n"
    ".text\n"
    "read:\n"
    "   li $v0, 4\n"
    "   la $a0, _prompt\n"
    "   syscall\n"
    "   li $v0, 5\n"
    "   syscall\n"
    "   jr $ra\n"
    "\n"
    "write:\n"
    "   li $v0, 1\n"
    "   syscall\n"
    "   li $v0, 4\n"
    "   la $a0, _ret\n"
    "   syscall\n"
    "   move $v0, $0\n"
    "   jr $ra\n";
    fprintf(f, "%s", s);
    while (tmp != end)
    {
        fprintf(f, "\n");
        tmp = tmp->next;
        while(tmp != end && tmp->kind != FUNCDEF)
            tmp = tmp->next;
        genMipsForFunc(funcBegin, tmp, f);
        funcBegin = tmp;
    }
    
}