
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
    int depth;
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

//For print error message
void semanticError(int type, int line, char *description);
//For copy a type
struct Type * copyType(struct Type *t);
//For hashtable operation
unsigned int symbolHash(char *name);
struct Symbol *search(char *name);
int insert(struct Symbol *s);
//For scope stack operation
struct Symbol *getStackTop();
void push();
void pop();
//For free memory
void freeFieldList(struct FieldList *f, int freeTypeFlag);
void freeSymbol(struct Symbol *s);
void freeType(struct Type *t);
//For check type and membership so on
int checkFieldEqual(struct FieldList *f1, struct FieldList *f2);
int checkTypeEqual(struct Type *t1, struct Type *t2);
void checkFuncArgs(struct TreeNode * practicalArgs, struct FieldList* formalArgs);
struct Type * checkStructField(struct FieldList *f, char *name);
//For traverse the syntax tree
struct Symbol *traverseForVarDec(struct TreeNode *p, struct Type *t);
struct FieldList *traverseForVarList(struct TreeNode *p);
struct FieldList *formFieldlist(struct TreeNode *p); // p is a deflist, and this func is used to travser the struct field
struct Type *traverseForSpecifier(struct TreeNode *p);
void traverseForDefList(struct TreeNode *p);
void traverseForExp(struct TreeNode *p);
void traverseForStmt(struct TreeNode *p);
void traverseForCompSt(struct TreeNode *p, int isPush);
void traverseForExtDef(struct TreeNode *p);
void traverse();
//For debug
void printFieldList(struct FieldList *f);
void printType(struct Type *t);
void printHashTable();


