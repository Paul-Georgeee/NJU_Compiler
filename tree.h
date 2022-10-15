struct Type;

struct TreeNode
{
    char name[32];
    union
    {
        int type_int;
        float type_float;
        char type_str[32];
        struct{
            struct Type *typeForExp;
            int leftFlag;
        }
    } value;
    int lineno;
    struct TreeNode *child;
    struct TreeNode *next;
};

struct TreeNode *root;
struct TreeNode *matchRule(char *father, struct TreeNode *child[], int num, int _lineno);