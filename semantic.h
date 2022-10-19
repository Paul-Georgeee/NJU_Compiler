
struct Type{
    enum {BASIC, ARRAY, STRUCTURE} kind;

    union{
        enum {T_INT, T_FLOAT} basic;
        struct {
            struct Type * elem;
            int size;
        } array;
        struct 
        {
            struct FieldList* field;
            char *name;
        }structure;
        
    };
};

//for struct and the args in function def
struct FieldList{
    char *name;
    struct Type * type;
    struct FieldList * next;
};

struct Symbol{
    enum {S_FUNC, S_VAR, S_STRUCT} flag;
    char *name;
    union 
    {
        struct Type *var;
        struct{
            struct Type *returnType;
            struct FieldList* args;
            int hasDef;
            int firstDeclareLine;
        } func;
        struct FieldList * structure;
        
    };
    // struct Symbol *hashLinkNext;
    struct Symbol *hashLinkNext, *hashLinkbefore;
    struct Symbol *blockLinkNext;
};


struct SymbolStack
{
    struct Symbol *stack[100];
    int tail;
};

int checkTypeEqual(struct Type *t1, struct Type *t2);
struct Type * traverseForSpecifier(struct TreeNode* p);
void traverseForCompSt(struct TreeNode *p);
struct Symbol * traverseForVarDec(struct TreeNode * p, struct Type *t);
void traverseForExp(struct TreeNode *p);
struct Symbol * search(char *name);
int insert(struct Symbol * s);