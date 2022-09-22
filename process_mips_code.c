#include "global.h"
#include "protos.h"
#include "syntax-tree.h"
#include "three-address-code.h"

#define HASHTBLSZ 256

extern symtabnode **globalTab;
extern symtabnode **localTab;
extern char *currentFuncName;
extern int returnType;

extern int hasInit;
extern int tmpVarCount;
extern int labelCount;
int isPrintChar = 0;

char *trueLabel = NULL;
char *falseLabel = NULL;


symtabnode *newTmpVar(int type) {
    char *name = malloc(10 * sizeof(char));
    sprintf(name, "_tmp%d", tmpVarCount);
    tmpVarCount++;
    symtabnode *tmp = SymTabInsert(name, Local);
    tmp->type = type;
    return tmp;
}

char *newLabelName() {
    char *name = malloc(10 * sizeof(char));
    sprintf(name, "L%d", labelCount);
    labelCount++;
    return name;
}

char bOrW(int type) {
    if (type == t_Char || type == t_Bool) {
        return 'b';
    } else {
        return 'w';
    }
}

void read(symtabnode *sym, char *reg) {
    char c = bOrW(sym->type);

    if (sym->scope == Global) {
        printf("  l%c %s, _%s \n", c, reg, sym->name);
    } else {
        printf("  l%c %s, %d($fp) \n", c, reg, sym->offset);
    }
}

void save(symtabnode *sym, char *reg) {
    char c = bOrW(sym->type);
    if (sym->scope == Global) {
        printf("  s%c %s, _%s \n", c, reg, sym->name);
    } else {
        printf("  s%c %s, %d($fp) \n", c, reg, sym->offset);
    }
}

void generateThreeAddressCode(tnode *t) {
    symtabnode *tmp;

    if (!t) {
        return;
    }

    switch (t->ntype) {
        case Assg:
            generateThreeAddressCode(stAssg_Lhs(t));
            generateThreeAddressCode(stAssg_Rhs(t));
            newAssignInstr(stAssg_Lhs(t)->val.strefNode.stptr, stAssg_Rhs(t)->val.strefNode.stptr);
            break;

        case Var:
            t->val.strefNode.stptr = stVar(t);
            break;

        case Intcon:
            tmp = newTmpVar(t->etype);
            newConstInstr(OpConstInt, tmp, t->val.iconst);
            t->val.strefNode.stptr = tmp;
            break;
        case Charcon:
            tmp = newTmpVar(t->etype);
            newConstInstr(OpConstChar, tmp, t->val.iconst);
            t->val.strefNode.stptr = tmp;
            break;
        case FunCall: {
            // run args first
            generateThreeAddressCode(stFunCall_Args(t));

            int NumParams = 0;
            for (tnode *param = stFunCall_Args(t); param != NULL; param = stList_Rest(param)) {
                NumParams++;
            }
            symtabnode *args[NumParams];
            int i = 0;
            for (tnode *param = stFunCall_Args(t); param != NULL; param = stList_Rest(param)) {
                args[i] = stList_Head(param)->val.strefNode.stptr;
                i++;
            }
            for (int i = 0; i < NumParams; i++) {
                newParamInstr(args[NumParams - i - 1]);
            }

            newCallInstr(stFunCall_Fun(t), NumParams);


            if (stFunCall_Fun(t)->ret_type != t_None) {
                tmp = newTmpVar(stFunCall_Fun(t)->ret_type);
                t->val.strefNode.stptr = tmp;
                newRetrieveInstr(tmp);
            }

            break;
        }
        case Return:
            if (stReturn(t)) {
                generateThreeAddressCode(stReturn(t));
                t->val.strefNode.stptr = stReturn(t)->val.strefNode.stptr;
            }
            newReturnInstr(returnType, t->val.strefNode.stptr);
            break;

        case STnodeList:
            for (tnode *tntmp0 = t; tntmp0 != NULL; tntmp0 = stList_Rest(tntmp0)) {
                generateThreeAddressCode(stList_Head(tntmp0));
            }
            break;
        case Stringcon:
            // TODO milestone 3
            break;
        case ArraySubscript:
            // TODO milestone 3
            break;
        case UnaryMinus:
            generateThreeAddressCode(stUnop_Op(t));
            tmp = newTmpVar(t->etype);
            newUnaryInstr(tmp, stUnop_Op(t)->val.strefNode.stptr);
            t->val.strefNode.stptr = tmp;
            break;
        case Plus:
            generateThreeAddressCode(stBinop_Op1(t));
            generateThreeAddressCode(stBinop_Op2(t));
            tmp = newTmpVar(t->etype);
            newExprInstr(OpPlus, stBinop_Op1(t)->val.strefNode.stptr, stBinop_Op2(t)->val.strefNode.stptr, tmp);
            t->val.strefNode.stptr = tmp;
            break;
        case BinaryMinus:
            generateThreeAddressCode(stBinop_Op1(t));
            generateThreeAddressCode(stBinop_Op2(t));
            tmp = newTmpVar(t->etype);
            newExprInstr(OpBinaryMinus, stBinop_Op1(t)->val.strefNode.stptr, stBinop_Op2(t)->val.strefNode.stptr, tmp);
            t->val.strefNode.stptr = tmp;
            break;
        case Mult:
            generateThreeAddressCode(stBinop_Op1(t));
            generateThreeAddressCode(stBinop_Op2(t));
            tmp = newTmpVar(t->etype);
            newExprInstr(OpMult, stBinop_Op1(t)->val.strefNode.stptr, stBinop_Op2(t)->val.strefNode.stptr, tmp);
            t->val.strefNode.stptr = tmp;
            break;
        case Div:
            generateThreeAddressCode(stBinop_Op1(t));
            generateThreeAddressCode(stBinop_Op2(t));
            tmp = newTmpVar(t->etype);
            newExprInstr(OpDiv, stBinop_Op1(t)->val.strefNode.stptr, stBinop_Op2(t)->val.strefNode.stptr, tmp);
            t->val.strefNode.stptr = tmp;
            break;
        case Equals:
            generateThreeAddressCode(stBinop_Op1(t));
            generateThreeAddressCode(stBinop_Op2(t));
            newTrueConditionInstr(OpEq, stBinop_Op1(t)->val.strefNode.stptr, stBinop_Op2(t)->val.strefNode.stptr,
                                  trueLabel);
            newGotoInstr(falseLabel);

            break;
        case Neq:
            generateThreeAddressCode(stBinop_Op1(t));
            generateThreeAddressCode(stBinop_Op2(t));
            newTrueConditionInstr(OpNeq, stBinop_Op1(t)->val.strefNode.stptr, stBinop_Op2(t)->val.strefNode.stptr,
                                  trueLabel);
            newGotoInstr(falseLabel);

            break;
        case Leq:
            generateThreeAddressCode(stBinop_Op1(t));
            generateThreeAddressCode(stBinop_Op2(t));
            newTrueConditionInstr(OpLeq, stBinop_Op1(t)->val.strefNode.stptr, stBinop_Op2(t)->val.strefNode.stptr,
                                  trueLabel);
            newGotoInstr(falseLabel);

            break;
        case Lt:
            generateThreeAddressCode(stBinop_Op1(t));
            generateThreeAddressCode(stBinop_Op2(t));
            newTrueConditionInstr(OpLt, stBinop_Op1(t)->val.strefNode.stptr, stBinop_Op2(t)->val.strefNode.stptr,
                                  trueLabel);
            newGotoInstr(falseLabel);

            break;
        case Geq:
            generateThreeAddressCode(stBinop_Op1(t));
            generateThreeAddressCode(stBinop_Op2(t));
            newTrueConditionInstr(OpGeq, stBinop_Op1(t)->val.strefNode.stptr, stBinop_Op2(t)->val.strefNode.stptr,
                                  trueLabel);
            newGotoInstr(falseLabel);

            break;
        case Gt:
            generateThreeAddressCode(stBinop_Op1(t));
            generateThreeAddressCode(stBinop_Op2(t));
            newTrueConditionInstr(OpGt, stBinop_Op1(t)->val.strefNode.stptr, stBinop_Op2(t)->val.strefNode.stptr,
                                  trueLabel);
            newGotoInstr(falseLabel);

            break;
        case LogicalAnd: {
            char *middleLabel = newLabelName();

            char *tmpLabel = trueLabel;
            trueLabel = middleLabel;
            generateThreeAddressCode(stBinop_Op1(t));

            trueLabel = tmpLabel;
            newLabelInstr(middleLabel);
            generateThreeAddressCode(stBinop_Op2(t));

            break;
        }
        case LogicalOr: {
            char *middleLabel = newLabelName();

            char *tmpLabel = falseLabel;
            falseLabel = middleLabel;
            generateThreeAddressCode(stBinop_Op1(t));

            falseLabel = tmpLabel;
            newLabelInstr(middleLabel);
            generateThreeAddressCode(stBinop_Op2(t));

            break;
        }
        case LogicalNot: {
            char *tmpLabel = trueLabel;
            trueLabel = falseLabel;
            falseLabel = tmpLabel;
            generateThreeAddressCode(stUnop_Op(t));

            break;
        }
        case For: {
            char *bodyLabel = newLabelName();
            char *conditionLabel = newLabelName();
            char *endLabel = newLabelName();

            // init
            generateThreeAddressCode(stFor_Init(t));

            newGotoInstr(conditionLabel);

            // body branch
            newLabelInstr(bodyLabel);
            generateThreeAddressCode(stFor_Body(t));
            generateThreeAddressCode(stFor_Update(t));

            // condition branch
            newLabelInstr(conditionLabel);
            trueLabel = bodyLabel;
            falseLabel = endLabel;
            if (stFor_Test(t)) {
                generateThreeAddressCode(stFor_Test(t));
            } else {
                // without condition, loop forever.
                newGotoInstr(bodyLabel);
            }

            // end branch
            newLabelInstr(endLabel);
            break;

        }
        case While: {
            char *bodyLabel = newLabelName();
            char *conditionLabel = newLabelName();
            char *endLabel = newLabelName();

            newGotoInstr(conditionLabel);

            // body branch
            newLabelInstr(bodyLabel);
            generateThreeAddressCode(stWhile_Body(t));
            newGotoInstr(conditionLabel);

            // condition branch
            newLabelInstr(conditionLabel);
            trueLabel = bodyLabel;
            falseLabel = endLabel;
            generateThreeAddressCode(stWhile_Test(t));

            // end branch
            newLabelInstr(endLabel);

            break;
        }
        case If: {
            char *thenLabel = newLabelName();
            char *elseLabel = newLabelName();
            char *endLabel = newLabelName();

            // has else
            if (stIf_Else(t)) {
                falseLabel = elseLabel;
                trueLabel = thenLabel;
                generateThreeAddressCode(stIf_Test(t));

                newLabelInstr(thenLabel);
                generateThreeAddressCode(stIf_Then(t));
                newGotoInstr(endLabel);

                newLabelInstr(elseLabel);
                generateThreeAddressCode(stIf_Else(t));
                newGotoInstr(endLabel);

            } else {
                // no else
                falseLabel = endLabel;
                trueLabel = thenLabel;
                generateThreeAddressCode(stIf_Test(t));

                newLabelInstr(thenLabel);
                generateThreeAddressCode(stIf_Then(t));
                newGotoInstr(endLabel);

            }

            newLabelInstr(endLabel);
            trueLabel = NULL;
            falseLabel = NULL;

            break;
        }
        default:
            break;
    }
}

void generateDataSeg() {
    symtabnode *stptr;
    printf("\n.data \n");
    for (int i = 0; i < HASHTBLSZ; i++) {
        for (stptr = globalTab[i]; stptr != NULL; stptr = stptr->next) {
            if (stptr->formal) {
                continue;
            }
            if (stptr->type == t_Int) {
                printf("_%s:.space 4 \n", stptr->name);
            } else if (stptr->type == t_Char) {
                printf("_%s:.space 1 \n", stptr->name);
                printf(".align 2 \n");
            }
        }
    }
    printf("\n.text \n");
}

void generateMipsCode(symtabnode *fn_name) {
    if (hasInit == 0) {
        printf(".align 2            \n");
        printf(".data               \n");
        printf("_nl: .asciiz \"\\n\"\n");
        printf(".align 2            \n");
        printf(".text               \n");
        printf("_println:           \n");
        printf("  li $v0, 1         \n");
        printf("  lw $a0, 0($sp)    \n");
        printf("  syscall           \n");
        printf("  li $v0, 4         \n");
        printf("  la $a0, _nl       \n");
        printf("  syscall           \n");
        printf("  jr $ra            \n");
        printf("_printlnChar:       \n");
        printf("  li $v0, 1         \n");
        printf("  lb $a0, 0($sp)    \n");
        printf("  syscall           \n");
        printf("  li $v0, 4         \n");
        printf("  la $a0, _nl       \n");
        printf("  syscall           \n");
        printf("  jr $ra            \n");

        printf("main: j _main       \n");
        hasInit = 1;
    }
    int varSpace = 0;
    int paramOffset = 8;

    for (int i = 0; i < HASHTBLSZ; i++) {
        for (symtabnode *stptr = localTab[i]; stptr != NULL; stptr = stptr->next) {
            if (!stptr->formal) {
                if (stptr->type == t_Int) {
                    varSpace += 4;
                } else if (stptr->type == t_Char) {
                    // varSpace += 1;
                    varSpace += 4;
                }
                stptr->offset = -varSpace;
            } else if (stptr->formal) {
                stptr->offset = paramOffset;
                paramOffset += 4;
            }
        }
    }

    char loadInst[3];
    char saveInst[3];

    instr *curr_instruction = instrHead;

    while (curr_instruction) {
        switch (curr_instruction->op) {
            case OpEnter:
                printf("\n.text \n");
                printf("_%s:              \n", curr_instruction->value.function->name);
                printf("  la $sp, -8($sp) # allocate space for old $fp and $ra\n");
                printf("  sw $fp, 4($sp)  # save old $fp\n");
                printf("  sw $ra, 0($sp)  # save return address\n");
                printf("  la $fp, 0($sp)  # set up frame pointer\n");
                printf("  la $sp, %d($sp) # allocate stack frame\n", -varSpace);
                break;

            case OpConstInt:
                printf("\n");
                printf("  # OpConstInt \n");
                printf("  li $t0, %d \n", curr_instruction->value.constant.value);
                printf("  sw $t0, %d($fp) \n", curr_instruction->value.constant.tmp->offset);
                break;

            case OpConstChar:
                printf("\n");
                printf("  # OpConstChar \n");
                printf("  li $t0, %d      \n", curr_instruction->value.constant.value);
                printf("  sb $t0, %d($fp) \n", curr_instruction->value.constant.tmp->offset);
                break;

            case OpAssign:
                printf("\n");
                printf("  # OpAssign \n");
                symtabnode *left = curr_instruction->value.assign.left;
                symtabnode *right = curr_instruction->value.assign.right;

                if (left->type == t_Char) {
                    strcpy(loadInst, "lb");
                    strcpy(saveInst, "sb");
                } else if (left->type == t_Int) {
                    strcpy(loadInst, "lw");
                    strcpy(saveInst, "sw");
                }

                if (right->scope == Global) {
                    printf("  %s $t0, _%s \n", loadInst, right->name);
                } else if (right->scope == Local) {
                    printf("  %s $t0, %d($fp) \n", loadInst, right->offset);
                }

                if (left->scope == Global) {
                    printf("  %s $t0, _%s \n", saveInst, left->name);
                } else if (left->scope == Local) {
                    printf("  %s $t0, %d($fp) \n", saveInst, left->offset);
                }

                break;
            case OpParam:
                printf("\n");
                printf("  # OpParam \n");

                if (curr_instruction->value.param->type == t_Char) {
                    strcpy(loadInst, "lb");
                    strcpy(saveInst, "sb");
                    isPrintChar = 1;
                } else if (curr_instruction->value.param->type == t_Int) {
                    strcpy(loadInst, "lw");
                    strcpy(saveInst, "sw");
                    isPrintChar = 0;
                }

                if (curr_instruction->value.param->scope == Global) {
                    printf("  %s $t0, _%s \n", loadInst, curr_instruction->value.param->name);
                } else {
                    printf("  %s $t0, %d($fp) \n", loadInst, curr_instruction->value.param->offset);
                }
                printf("  la $sp, -4($sp)  \n");
                printf("  %s $t0, 0($sp)   \n", saveInst);
                break;
            case OpCall:
                printf("\n");
                printf("  # OpCall       \n");
                if (!strcmp(curr_instruction->value.call.callee->name, "println")) {
                    if (isPrintChar) {
                        printf("  jal _%s         \n", "printlnChar");
                    } else {
                        printf("  jal _%s         \n", "println");
                    }
                } else {
                    printf("  jal _%s         \n", curr_instruction->value.call.callee->name);
                }
                printf("  la $sp, %d($sp) \n", 4 * curr_instruction->value.call.numOfParams);
                break;

            case OpReturn: {
                printf("\n");
                printf("  # OpReturn \n");
                if (curr_instruction->value.returnStr.returnVar) {
                    read(curr_instruction->value.returnStr.returnVar, "$v0");
                }
                printf("  la $sp, 0($fp) # deallocate locals \n");
                printf("  lw $ra, 0($sp) # restore return address \n");
                printf("  lw $fp, 4($sp) # restore frame pointer \n");
                printf("  la $sp, 8($sp) # restore stack pointer \n");
                printf("  jr $ra         # return\n");

                if (!strcmp(fn_name->name, "main")) {
                    generateDataSeg();
                }
                break;
            }
            case OpRetrieve:
                save(curr_instruction->value.retrieve, "$v0");
                break;
            case OpUnaryMinus:
                printf("\n");
                printf("  # OpUMinus    \n");
                // Load the content from src1 into $t0
                read(curr_instruction->value.assign.right, "$t0");

                printf("  neg $t1, $t0 \n");
                save(curr_instruction->value.assign.left, "$t1");
                break;
            case OpPlus:
                printf("\n");
                printf("  # OpPlus    \n");
                read(curr_instruction->value.expr.src1, "$t0");
                read(curr_instruction->value.expr.src2, "$t1");
                printf("  add $t2, $t0, $t1 \n");
                save(curr_instruction->value.expr.dest, "$t2");
                break;
            case OpBinaryMinus:
                printf("\n");
                printf("  # OpBinaryMinus    \n");
                read(curr_instruction->value.expr.src1, "$t0");
                read(curr_instruction->value.expr.src2, "$t1");
                printf("  sub $t2, $t0, $t1 \n");
                save(curr_instruction->value.expr.dest, "$t2");
                break;
            case OpMult:
                printf("\n");
                printf("  # OpMult    \n");
                read(curr_instruction->value.expr.src1, "$t0");
                read(curr_instruction->value.expr.src2, "$t1");
                printf("  mul $t2, $t0, $t1 \n");
                save(curr_instruction->value.expr.dest, "$t2");
                break;
            case OpDiv:
                printf("\n");
                printf("  # OpDiv   \n");
                read(curr_instruction->value.expr.src1, "$t0");
                read(curr_instruction->value.expr.src2, "$t1");
                printf("  div $t2, $t0, $t1 \n");
                save(curr_instruction->value.expr.dest, "$t2");
                break;
            case OpLabel:
                printf("\n");
                printf("  # opLabel \n");
                printf("  _%s:       \n", curr_instruction->value.label);
                break;
            case OpGoto:
                printf("\n");
                printf("  # opGoto \n");
                printf("  j _%s     \n", curr_instruction->value.gotoLabel);
                break;
            case OpIf:
                printf("\n");
                printf("  # opIf \n");
                read(curr_instruction->value.condition.cond1, "$t0");
                read(curr_instruction->value.condition.cond2, "$t1");
                switch (curr_instruction->value.condition.logiOp) {
                    case OpEq:
                        printf("  beq $t0, $t1, _%s \n", curr_instruction->value.condition.trueLabel);
                        break;
                    case OpNeq:
                        printf("  bne $t0, $t1, _%s \n", curr_instruction->value.condition.trueLabel);
                        break;
                    case OpLeq:
                        printf("  ble $t0, $t1, _%s \n", curr_instruction->value.condition.trueLabel);
                        break;
                    case OpLt:
                        printf("  blt $t0, $t1, _%s \n", curr_instruction->value.condition.trueLabel);
                        break;
                    case OpGeq:
                        printf("  bge $t0, $t1, _%s \n", curr_instruction->value.condition.trueLabel);
                        break;
                    case OpGt:
                        printf("  bgt $t0, $t1, _%s \n", curr_instruction->value.condition.trueLabel);
                        break;
                }
            default:
                break;
        }
        curr_instruction = curr_instruction->next;
    }
}


