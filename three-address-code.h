#include "symbol-table.h"
#include "global.h"

typedef enum {
    OpEnter,
    OpAssign,
    OpConstInt,
    OpConstChar,
    OpCall,
    OpParam,
    OpReturn
}
operator;

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
        symtabnode *function;
        symtabnode *param;
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

void newReturnInstr();

void newParamInstr(symtabnode *param);