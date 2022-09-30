#include "symbol-table.h"
#include "global.h"

typedef enum {
    OpEnter,
    OpAssign,
    OpConstInt,
    OpConstChar,
    OpCall,
    OpParam,
    OpRetrieve,
    OpReturn,
    OpUnaryMinus,
    OpPlus,
    OpBinaryMinus,
    OpMult,
    OpDiv,
    OpGoto,
    OpIf,
    OpLabel,
    OpArray,
    OpDeref
}
operator;

typedef enum {
    OpEq,
    OpNeq,
    OpLeq,
    OpLt,
    OpGeq,
    OpGt
} logicalOp;

typedef struct instr {
    operator op;
    union {
        struct {
            symtabnode *tmp;
            int value;
        } constant;
        struct {
            char *name;
            int type;
        } global;
        struct {
            symtabnode *callee;
            int numOfParams;
        } call;
        struct {
            symtabnode *left;
            symtabnode *right;
        } assign;
        struct {
            symtabnode *dest;
            symtabnode *src1;
            symtabnode *src2;
        } expr;
        struct {
            symtabnode *cond1;
            symtabnode *cond2;
            logicalOp logiOp;
            char *trueLabel;
        } condition;
        struct {
            symtabnode *arr;
            symtabnode *subscript;
            symtabnode *address;
        }array;
        symtabnode *returnVar;
        char *gotoLabel;
        char *label;
        symtabnode *function;
        symtabnode *param;
        symtabnode *retrieve;
    } value;

    struct instr *next;
} instr;

extern instr *instrHead;
extern instr *instrTail;

void newConstInstr(operator op, symtabnode * tmp, int
value);

void newCallInstr(symtabnode *callee, int numOfParams);

void newEnterInstr(symtabnode *function);

void newAssignInstr(symtabnode *left, symtabnode *right);

void newReturnInstr(symtabnode *returnVal);

void newRetrieveInstr(symtabnode *retrieve);

void newParamInstr(symtabnode *param);

void newUnaryInstr(symtabnode *left, symtabnode *right);

void newExprInstr(operator op, symtabnode * src1, symtabnode * src2, symtabnode * dest);

void newGotoInstr(char *label);

void newLabelInstr(char *label);

void newTrueConditionInstr(logicalOp logiOp, symtabnode *cond1, symtabnode *cond2, char *trueLabel);

void newArrayInstr(symtabnode *arrayName, symtabnode *subscript,symtabnode *arrayAddress);

void newDerefInstr(symtabnode *left, symtabnode *right);

