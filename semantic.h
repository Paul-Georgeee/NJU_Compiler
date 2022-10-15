
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
            int argn;
            struct FieldList* args;
        } func;
        struct FieldList * structure;
        
    };
    struct Symbol *hashLinkNext;
    // struct Symbol *hashLinkNext, *hashLinkbefore;
    // struct Symbol *blockLinkNext;
};


struct SymbolStack
{
    struct Symbol *stack[100];
    int tail;
};

struct Type * traverseForSpecifier(struct TreeNode* p);
struct Symbol * traverseForVarDec(struct TreeNode * p, struct Type *t);

struct Symbol * search(char *name);
void insert(struct Symbol * s);