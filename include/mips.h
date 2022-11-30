struct FuncFrame{
    int hasFunCall;
    int returnAdd;      //offset by the fp where the return address saved in stack
    int size;
}funcFrame;

struct Reg{
    int contain; //indicate whether there is a var in register
    struct Operand *op;
};

struct Reg regs[32];

void genMips(struct InterCode* begin, struct InterCode* end, FILE *f);
