struct Operand{
    enum OperandKind {BASCIVAR, REFVAR, CONSTANT_INT, CONSTANT_FLOAT, ADDRESS, LABEL} kind;
    union {
        int constantValueInt;
        float constantValueFloat;
        char *varName;
    };
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
struct InterCode{
    enum InterCodeKind kind;    
    union 
    {
        struct {struct Operand* right, *left;} unaryop; //include assign, reference(&), dereference(*)  DEC x [size], x := call f
        struct {struct Operand* result, *op1, *op2;} binaryop; //include + - * / < > == <= >= !=, IF x relop y GOTO z, 
        struct {struct Operand* op;} noresult; //include LABEL x, FUNCTION f, GOTO x, RETURN x, ARG x, PARAM x, READ x, WRITE x 
    };
    
    struct InterCode *prev, *next;
};


struct IRList
{
    struct InterCode *head;
    struct InterCode *tail;
}irList;

void initIRGenerator();
void insertIRList(struct InterCode * p);
struct InterCode* genUnaryop(enum InterCodeKind k, struct Operand* right, struct Operand* left);
struct InterCode* genBinaryop(enum InterCodeKind k, struct Operand* result, struct Operand* op1, struct Operand* op2);
struct InterCode* genNoresult(enum InterCodeKind k, struct Operand* op);

struct Operand* genConstInt(int value);
struct Operand* genConstFloat(float value);
struct Operand* genBasicVariable(char *name);

void translateExp(struct TreeNode* exp, struct Operand* place);
void translateCond(struct TreeeNode* exp, struct Operand* labelTrue, struct Operand *labelFalse);
