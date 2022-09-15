#ifndef _THREE_ADDRESS_CODE_H_
#define _THREE_ADDRESS_CODE_H_

#include "symbol-table.h"
#include "global.h"

typedef enum operator
{
    OpGlobal,
    OpEnter,
    OpAssign,
    OpConstInt,
    OpConstChar,
    OpCall,
    OpParam,
    OpReturn
}
operator;

typedef struct instr
{
    operator op;
    symtabnode *dest;

    union
    {
        struct opMember
        {
            symtabnode *src1;
            symtabnode *src2;
        } opMember;
        struct global
        {
            char *name;
            int type;
        } global;
        struct call
        {
            symtabnode *callee;
            int numOfParams;
        } call;
        int constant;
    } value;

    struct instr *next;
} instr;

void appendInstructions(instr *instruction);

void *newInstr(operator op, symtabnode *dest, symtabnode *src1, symtabnode *src2);
void *newConstInstr(operator op, symtabnode *dest, int value);
void *newGlobalInstr(operator op, char *name, int type);
void *newCallInstr(symtabnode *callee, int numOfParams);

instr *newParamInstr(operator op, symtabnode *src1);

#endif