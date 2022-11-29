struct CFGNode{
    struct InterCode *begin;
    struct InterCode *end;
    int statementCnt;
    struct AdjList{
        int index;
        struct AdjList *next;
    }succ, prev;
};
struct Edge{
    struct InterCode *from;
    struct InterCode *to;
};

#define MAXCFGNode 1000
#define MAXEdge 4000
struct CFG{
    struct CFGNode nodes[MAXCFGNode];
    struct Edge edges[MAXEdge];
    int nodeCnt;
    int edgeCnt;
    int enrtyIndex;
    int exitIndex;
} cfg;



void constructCFG(struct InterCode *codeBegin, struct InterCode *codeEnd);
void printCFG();