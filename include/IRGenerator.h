struct Operand{
    enum {VARIABLE, CONSTANT, ADDRESS} kind;
    union {
        int value;
        int var_no;
    }u;
};