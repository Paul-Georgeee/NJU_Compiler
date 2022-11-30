enum OperandKind {BASCIVAR, REFVAR, CONSTANT_INT, CONSTANT_FLOAT, ADDRESS, LABEL, FUNC};
//BASCIVAR: int float
//REFVAR: struct array

#define NOTINREG -1
#define NOTINMEM 0xffffffff
struct Operand{
    enum OperandKind kind;
    union {
        int constantValueInt;
        float constantValueFloat;
        char *name;
    };
    int cnt;    //used for avoiding memory leak
    struct Operand *next;

    int regIndex;
    int offsetByfp;
    int needLoad;
    struct UseList{
        struct InterCode* code;
        struct UseList* next;
    }useList;
};
enum InterCodeKind{
        ASSIGN,
        PLUS,           // +
        MINUS,          // -
        STAR,           // *
        DIV, 
        REF,            // a = &b
        L_DEREF,        // *a = b
        R_DEREF,        // a = *b
        MALLOC,         // DEC x [size]
        FUNCALL,
        IF,             // IF x [relop] y GOTO z
        LABELDEF,       // LABEL x
        FUNCDEF,        // FUNCTION f
        GOTO,           // GOTO x
        RETURN,
        ARG,            // ARG x
        PARAM,          // PARAM x
        READ,
        WRITE
};

enum RELOP {EQ, NE, LT, GT, LE, GE};  // == != < > <= >=
struct InterCode{
    enum InterCodeKind kind;    
    union 
    {
        struct {struct Operand* right, *left;} unaryop; //include assign, reference(&), dereference(*)  DEC x [size], x := call f
        struct {struct Operand* result, *op1, *op2;} binaryop; //include + - * / 
        struct {struct Operand* op;} noresult; //include LABEL x, FUNCTION f, GOTO x, RETURN x, ARG x, PARAM x, READ x, WRITE x 
        struct {
            struct Operand* result, *op1, *op2;
            enum RELOP relop; 
        } ifop;
    };
    
    struct InterCode *prev, *next;
    
    int mallocAddr;
    int no;
    struct CFGInfo{
        int firstInstr;
        int basicBlockIndex;
    }cfgInfo;

};


struct IRList
{
    struct InterCode *head;
    struct InterCode *tail;
}irList;

void initIRGenerator();
void insertIRList(struct InterCode * p);

void genUnaryop(enum InterCodeKind k, struct Operand* right, struct Operand* left);
void genBinaryop(enum InterCodeKind k, struct Operand* result, struct Operand* op1, struct Operand* op2);
void genNoresult(enum InterCodeKind k, struct Operand* op);
void genIfop(struct Operand* result, struct Operand* op1, struct Operand* op2, char *relop);

struct Operand* genConstInt(int value);
struct Operand* genConstFloat(float value);
struct Operand* genVariable(char *name, enum OperandKind k);

void translateArgs(struct TreeNode* args, struct FieldList *formalArgs, int isWriteFunc);
void translateExp(struct TreeNode* exp, struct Operand* place);
void translateCond(struct TreeNode* exp, struct Operand* labelTrue, struct Operand *labelFalse);
void translateFunDec(struct TreeNode *fundec);
void translateCompst(struct TreeNode *compst);
void translateDef(struct TreeNode* def);
void translateStmt(struct TreeNode *stmt);
struct Operand * translateRef(struct TreeNode* exp);

void printIR(FILE *fp, struct InterCode* begin, struct InterCode* end);
void freeIRList();

int calculateRefsize(struct Type *t);
