#include "global.h"
#include "protos.h"
#include "syntax-tree.h"
#include "three-address-code.h"

#define HASHTBLSZ 256

extern symtabnode **globalTab;
extern symtabnode **localTab;
extern char *currentFuncName;
extern int hasInit;
extern int tmpVarCount;
int isPrintChar = 0;

symtabnode *newTmpVar(int type) {
    char name[10] = "_tmp";
    sprintf(name, "_tmp%d", tmpVarCount);
    tmpVarCount++;
    symtabnode *tmp = SymTabInsert(name, Local);
    tmp->type = type;
    return tmp;
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

            break;
        }


        case Return:
            newReturnInstr();
            break;

        case STnodeList:
            for (tnode *tntmp0 = t; tntmp0 != NULL; tntmp0 = stList_Rest(tntmp0)) {
                generateThreeAddressCode(stList_Head(tntmp0));
            }
            break;

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
                printf("%s:.space 4 \n", stptr->name);
            } else if (stptr->type == t_Char) {
                printf("%s:.space 1 \n", stptr->name);
                printf(".align 2 \n");
            }
        }
    }
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
                    printf("  %s $t0, %s \n", loadInst, right->name);
                } else if (right->scope == Local) {
                    printf("  %s $t0, %d($fp) \n", loadInst, right->offset);
                }

                if (left->scope == Global) {
                    printf("  %s $t0, %s \n", saveInst, left->name);
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
                    printf("  %s $t0, %s \n", loadInst, curr_instruction->value.param->name);
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

            default:
                break;
        }
        curr_instruction = curr_instruction->next;
    }
}


