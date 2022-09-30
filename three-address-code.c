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

void newReturnInstr(symtabnode *returnVal) {
    instr *instruction = zalloc(sizeof(*instruction));
    instruction->op = OpReturn;
    instruction->value.returnVar = returnVal;
    appendInstruction(instruction);
}

void newRetrieveInstr(symtabnode *retrieve) {
    instr *instruction = zalloc(sizeof(*instruction));
    instruction->op = OpRetrieve;
    instruction->value.retrieve = retrieve;
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

void newUnaryInstr(symtabnode *left, symtabnode *right) {
    instr *instruction = zalloc(sizeof(*instruction));
    instruction->op = OpUnaryMinus;
    instruction->value.assign.left = left;
    instruction->value.assign.right = right;
    appendInstruction(instruction);
}

void newExprInstr(operator op, symtabnode *src1, symtabnode *src2, symtabnode *dest) {
    instr *instruction = zalloc(sizeof(*instruction));
    instruction->op = op;
    instruction->value.expr.src1 = src1;
    instruction->value.expr.src2 = src2;
    instruction->value.expr.dest = dest;
    appendInstruction(instruction);
}

void newGotoInstr(char *label) {
    instr *instruction = zalloc(sizeof(*instruction));
    instruction->op = OpGoto;
    instruction->value.gotoLabel = label;
    appendInstruction(instruction);
}

void newTrueConditionInstr(logicalOp logiOp, symtabnode *cond1, symtabnode *cond2, char *trueLabel) {
    instr *instruction = zalloc(sizeof(*instruction));
    instruction->op = OpIf;
    instruction->value.condition.cond1 = cond1;
    instruction->value.condition.cond2 = cond2;
    instruction->value.condition.logiOp = logiOp;
    instruction->value.condition.trueLabel = trueLabel;

    appendInstruction(instruction);
}

void newLabelInstr(char *label) {
    instr *instruction = zalloc(sizeof(*instruction));
    instruction->op = OpLabel;
    instruction->value.label = label;
    appendInstruction(instruction);
}

void newArrayInstr(symtabnode *arrayName, symtabnode *subscript,symtabnode *arrayAddress) {
    instr *instruction = zalloc(sizeof(*instruction));
    instruction->op = OpArray;
    instruction->value.array.arr = arrayName;
    instruction->value.array.subscript = subscript;
    instruction->value.array.address = arrayAddress;
    appendInstruction(instruction);
}

void newDerefInstr(symtabnode *left, symtabnode *right) {
    instr *instruction = zalloc(sizeof(*instruction));
    instruction->op = OpDeref;
    instruction->value.assign.left = left;
    instruction->value.assign.right = right;
    appendInstruction(instruction);
}


