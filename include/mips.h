struct FuncFrame{
    int hasFunCall;
    int returnAdd;
    int size;
}funcFrame;

struct Reg{
    int contain;
    struct Operand *op;
};

struct Reg regs[32];

void genMips(struct InterCode* begin, struct InterCode* end, FILE *f);