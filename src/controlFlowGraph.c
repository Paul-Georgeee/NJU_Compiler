#include <stdio.h>
#include "tree.h"
#include "semantic.h"
#include "IRGenerator.h"
#include "controlFlowGraph.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>


struct InterCode* findLabel(struct Operand * label, struct InterCode* codeBegin, struct InterCode* codeEnd)
{
    assert(label->kind == LABEL);
    struct InterCode* tmp = codeBegin;
    while (tmp != codeEnd)
    {
        if(tmp->kind == LABELDEF && tmp->noresult.op == label)
            return tmp;        
        tmp = tmp->next;
    }
    assert(0);
}

void addSucc(struct CFGNode *node, int succ)
{
    struct AdjList *adj = (struct AdjList *)malloc(sizeof(struct AdjList));
    memset(adj, 0, sizeof(struct AdjList));
    adj->index = succ;
    adj->next = node->succ.next;
    node->succ.next = adj;
}

void addPrev(struct CFGNode *node, int prev)
{
    struct AdjList *adj = (struct AdjList *)malloc(sizeof(struct AdjList));
    memset(adj, 0, sizeof(struct AdjList));
    adj->index = prev;
    adj->next = node->prev.next;
    node->prev.next = adj;
}

void addEntry()
{
    cfg.enrtyIndex = cfg.nodeCnt;
    cfg.nodes[cfg.nodeCnt].begin = cfg.nodes[cfg.nodeCnt].end = NULL;
    cfg.nodes[cfg.nodeCnt].statementCnt = 0;
    cfg.nodeCnt = cfg.nodeCnt + 1;
    
}

void addExit()
{
    cfg.exitIndex = cfg.nodeCnt;
    cfg.nodes[cfg.nodeCnt].begin = cfg.nodes[cfg.nodeCnt].end = NULL;
    cfg.nodes[cfg.nodeCnt].statementCnt = 0;
    cfg.nodeCnt = cfg.nodeCnt + 1;
}

void addNode(struct InterCode *begin, struct InterCode *end, int count)
{
    cfg.nodes[cfg.nodeCnt].statementCnt = count;
    cfg.nodes[cfg.nodeCnt].begin = begin;
    cfg.nodes[cfg.nodeCnt].end = end;
    cfg.nodeCnt = cfg.nodeCnt + 1;
}

void addEdge(struct InterCode *from, struct InterCode *to)
{
    cfg.edges[cfg.edgeCnt].from = from;
    cfg.edges[cfg.edgeCnt].to = to;
    cfg.edgeCnt = cfg.edgeCnt + 1;
}

void constructCFG(struct InterCode *codeBegin, struct InterCode *codeEnd)
{
    memset(&cfg, 0, sizeof(struct CFG));
    struct InterCode *tmp = codeBegin;
    tmp->cfgInfo.firstInstr = 1;
    addEdge(NULL, tmp);
    while (tmp != codeEnd)
    {
        if(tmp->kind == GOTO)
        {
            if(tmp->next != codeEnd)
                tmp->next->cfgInfo.firstInstr = 1;
            struct InterCode * labelDef = findLabel(tmp->noresult.op, codeBegin, codeEnd);
            labelDef->cfgInfo.firstInstr = 1;
            addEdge(tmp, labelDef);
        }
        else if(tmp->kind == IF)
        {
            if(tmp->next != codeEnd)
                tmp->next->cfgInfo.firstInstr = 1;
            struct InterCode * labelDef = findLabel(tmp->ifop.result, codeBegin, codeEnd);
            labelDef->cfgInfo.firstInstr = 1;
            addEdge(tmp, labelDef);
        }
        else if(tmp->kind == RETURN)
        {
            if(tmp->next != codeEnd)
                tmp->next->cfgInfo.firstInstr = 1;
            addEdge(tmp, NULL);
        }
        tmp = tmp->next;
    }

    addEntry();
    struct InterCode* firstInstr = NULL;
    int cnt = 0;
    for(tmp = codeBegin; tmp != codeEnd; tmp = tmp->next)
    {
        if(tmp->cfgInfo.firstInstr == 1)
        {
            if(firstInstr != NULL)
                addNode(firstInstr, tmp, cnt);
            cnt = 0;
            firstInstr = tmp;
            if(tmp != codeBegin && tmp->prev->kind != GOTO && tmp->prev->kind != RETURN)
                addEdge(tmp->prev, tmp);
        }
        cnt = cnt + 1;
        tmp->cfgInfo.basicBlockIndex = cfg.nodeCnt;
    }
    addNode(firstInstr, codeEnd, cnt);
    
    addExit();
    
    for(int i = 0; i < cfg.edgeCnt; ++i)
    {
        if(cfg.edges[i].from == NULL)
        {
            addSucc(cfg.nodes + cfg.enrtyIndex, cfg.edges[i].to->cfgInfo.basicBlockIndex);
            addPrev(cfg.nodes + cfg.edges[i].to->cfgInfo.basicBlockIndex, cfg.enrtyIndex);
        }
        else if(cfg.edges[i].to == NULL)
        {
            addSucc(cfg.nodes + cfg.edges[i].from->cfgInfo.basicBlockIndex, cfg.exitIndex);
            addPrev(cfg.nodes + cfg.exitIndex, cfg.edges[i].from->cfgInfo.basicBlockIndex);
        }
        else
        {
            int fromIndex = cfg.edges[i].from->cfgInfo.basicBlockIndex, toIndex = cfg.edges[i].to->cfgInfo.basicBlockIndex;
            addSucc(cfg.nodes + fromIndex, toIndex);
            addPrev(cfg.nodes + toIndex, fromIndex);
        }
    }
}

void printCFG()
{
    for(int i = 0; i < cfg.nodeCnt; ++i)
    {
        printf("%d:\n", i);
        if(i == cfg.enrtyIndex)
            printf("enrty\n");
        else if(i == cfg.exitIndex)
            printf("exit\n");
        else    
            printIR(stdout, cfg.nodes[i].begin, cfg.nodes[i].end);
    }
    printf("Succ:\n");
    for(int i = 0; i < cfg.nodeCnt; ++i)
    {
        printf("%d's Edges:\n", i);
        struct AdjList * adj = cfg.nodes[i].succ.next;
        while (adj != NULL)
        {
            printf("%d -> %d\n", i, adj->index);
            adj = adj->next;
        }
    }
    printf("Prev:\n");
    for(int i = 0; i < cfg.nodeCnt; ++i)
    {
        printf("%d's Edges:\n", i);
        struct AdjList * adj = cfg.nodes[i].prev.next;
        while (adj != NULL)
        {
            printf("%d -> %d\n", i, adj->index);
            adj = adj->next;
        }
    }
}