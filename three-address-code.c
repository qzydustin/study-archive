#include "three-address-code.h"
extern instr *instrHead;
extern instr *instrTail;
extern instr *globalTail;

void appendInstruction(instr *instruction)
{
    if (!instrTail)
    {
        instrTail = instruction;
        instrHead = instruction;
    }
    else
    {
        instrTail->next = instruction;
        instrTail = instrTail->next;
    }
}

void appendInstructions(instr *instructions)
{
    instr *current = instructions;
    while (current)
    {
        appendInstruction(current);
        current = current->next;
    }
}

void insertGlobalInstruction(instr *instruction)
{
    if (!instrHead)
    {
        instrHead = instruction;
        instrTail = instruction;
        globalTail = instruction;
    }
    else if (!globalTail)
    {
        instruction->next = instrHead;
        instrHead = instruction;
        globalTail = instruction;
    }
    else
    {
        instruction->next = globalTail->next;
        globalTail->next = instruction;
        globalTail = instruction;
    }
}

void *newInstr(operator op, symtabnode *dest, symtabnode *src1, symtabnode *src2)
{

    instr *instruction = zalloc(sizeof(*instruction));

    instruction->op = op;
    instruction->value.opMember.src1 = src1;
    instruction->value.opMember.src2 = src2;
    instruction->dest = dest;

    appendInstruction(instruction);
}

void *newConstInstr(operator op, symtabnode *dest, int value)
{
    instr *instruction = zalloc(sizeof(*instruction));
    instruction->op = op;
    instruction->dest = dest;
    instruction->value.constant = value;
    appendInstruction(instruction);
}

void *newGlobalInstr(operator op, char *name, int type)
{
    instr *instruction = zalloc(sizeof(*instruction));
    instruction->op = op;
    instruction->value.global.name = name;
    instruction->value.global.type = type;
    insertGlobalInstruction(instruction);
}

void *newCallInstr(symtabnode *callee, int numOfParams)
{
    instr *instruction = zalloc(sizeof(*instruction));
    instruction->op = OpCall;
    instruction->value.call.callee = callee;
    instruction->value.call.numOfParams = numOfParams;
    appendInstruction(instruction);
}

instr *newParamInstr(operator op, symtabnode *src1)
{
    instr *instruction = zalloc(sizeof(*instruction));
    instruction->op = op;
    instruction->value.opMember.src1 = src1;
    return instruction;
}
