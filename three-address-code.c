#include "three-address-code.h"

void appendInstruction(instr *instruction) {
    if (!instrTail) {
        instrTail = instruction;
        instrHead = instruction;
    } else {
        instrTail->next = instruction;
        instrTail = instrTail->next;
    }
}

void newConstInstr(operator op, symtabnode *tmp, int value) {
    instr *instruction = zalloc(sizeof(*instruction));
    instruction->op = op;
    instruction->value.constant.tmp = tmp;
    instruction->value.constant.value = value;
    appendInstruction(instruction);
}

void newEnterInstr(symtabnode *function) {
    instr *instruction = zalloc(sizeof(*instruction));
    instruction->op = OpEnter;
    instruction->value.function = function;
    appendInstruction(instruction);
}

void newReturnInstr() {
    instr *instruction = zalloc(sizeof(*instruction));
    instruction->op = OpReturn;
    appendInstruction(instruction);
}

void newAssignInstr(symtabnode *left, symtabnode *right) {
    instr *instruction = zalloc(sizeof(*instruction));
    instruction->op = OpAssign;
    instruction->value.assign.left = left;
    instruction->value.assign.right = right;
    appendInstruction(instruction);
}

void newParamInstr(symtabnode *param) {
    instr *instruction = zalloc(sizeof(*instruction));
    instruction->op = OpParam;
    instruction->value.param = param;
    appendInstruction(instruction);
}

void newCallInstr(symtabnode *callee, int numOfParams) {
    instr *instruction = zalloc(sizeof(*instruction));
    instruction->op = OpCall;
    instruction->value.call.callee = callee;
    instruction->value.call.numOfParams = numOfParams;
    appendInstruction(instruction);
}
