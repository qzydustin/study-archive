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
extern int labelCount;
extern int stringCount;
int isPrintChar = 0;

char *trueLabel = NULL;
char *falseLabel = NULL;


struct {
    symtabnode *head;
    symtabnode *tail;
} stringList;

symtabnode *newTmpVar(int type) {
    char *name = malloc(10 * sizeof(char));
    sprintf(name, "_tmp%d", tmpVarCount);
    tmpVarCount++;
    symtabnode *tmp = SymTabInsert(name, Local);
    tmp->type = type;
    return tmp;
}

void addStringList(symtabnode *tmp) {
    if (stringList.tail) {
        stringList.tail->next = tmp;
        stringList.tail = tmp;
    } else {
        stringList.head = tmp;
        stringList.tail = tmp;
    }
}

symtabnode *newTmpStr(char *str) {
    symtabnode *tmp = (symtabnode *) zalloc(sizeof(symtabnode));
    tmp->type = t_Array;
    tmp->elt_type = t_Char;
    tmp->scope = Global;
    tmp->name = malloc(10 * sizeof(char));
    sprintf(tmp->name, "_str%d", stringCount++);
    tmp->string = malloc(strlen(str) * sizeof(char));
    tmp->string = strcpy(tmp->string, str);
    addStringList(tmp);

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
        isPrintChar = 1;
        return 'b';
    } else if (type == t_Int){
        isPrintChar = 0;
        return 'w';
    } else if (type == t_Array){
        return 'a';
    }
}

void read(symtabnode *sym, char *reg) {
    char c = bOrW(sym->type);

    if (sym->scope == Global) {
        printf("  l%c %s, _%s \n", c, reg, sym->name);
    } else if(sym->scope == Local){
        printf("  l%c %s, %d($fp) \n", c, reg, sym->offset);
    }
}

void save(symtabnode *sym, char *reg) {
    char c = bOrW(sym->type);
    if (sym->scope == Global) {
        printf("  s%c %s, _%s \n", c, reg, sym->name);
    } else if (sym->scope == Local){
        printf("  s%c %s, %d($fp) \n", c, reg, sym->offset);
    }
}

void readString(symtabnode *sym, char *reg, char c) {
    if (sym->scope == Global) {
        printf("  l%c %s, _%s \n", c, reg, sym->name);
    } else if(sym->scope == Local){
        printf("  l%c %s, %d($fp) \n", c, reg, sym->offset);
    }
}

void saveString(symtabnode *sym, char *reg, char c) {
    if (sym->scope == Global) {
        printf("  s%c %s, _%s \n", c, reg, sym->name);
    } else if(sym->scope == Local){
        printf("  s%c %s, %d($fp) \n", c, reg, sym->offset);
    }
}




void generateThreeAddressCode(tnode *t, int isLeftValue) {
    symtabnode *tmp;

    if (!t) {
        return;
    }
    switch (t->ntype) {
        case Assg:
            generateThreeAddressCode(stAssg_Lhs(t),1);
            generateThreeAddressCode(stAssg_Rhs(t),0);

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
        case Stringcon:
            t->val.strefNode.stptr = newTmpStr(stStringcon(t));
            break;
        case ArraySubscript:
            generateThreeAddressCode(stArraySubscript_Subscript(t),0);
            symtabnode *array_node = stArraySubscript_Array(t);

            symtabnode *tmp = newTmpVar(t_Array);
            tmp->elt_type = array_node->elt_type;


            newArrayInstr(stArraySubscript_Subscript(t)->val.strefNode.stptr, array_node, tmp);


            if (isLeftValue == 1) {
                t->val.strefNode.stptr = tmp;
            } else {
//                printf("ENTER RIGHT\n");
                t->val.strefNode.stptr = newTmpVar(array_node->elt_type);
                newDerefInstr(t->val.strefNode.stptr, tmp);
            }


            break;
        case FunCall: {
            // run args first
            generateThreeAddressCode(stFunCall_Args(t),0);

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
                generateThreeAddressCode(stReturn(t),0);
                t->val.strefNode.stptr = stReturn(t)->val.strefNode.stptr;
            }
            newReturnInstr(t->val.strefNode.stptr);
            break;

        case STnodeList:
            for (tnode *tntmp0 = t; tntmp0 != NULL; tntmp0 = stList_Rest(tntmp0)) {
                generateThreeAddressCode(stList_Head(tntmp0),isLeftValue);
            }
            break;
        case UnaryMinus:
            generateThreeAddressCode(stUnop_Op(t),0);
            tmp = newTmpVar(t->etype);
            newUnaryInstr(tmp, stUnop_Op(t)->val.strefNode.stptr);
            t->val.strefNode.stptr = tmp;
            break;
        case Plus:
            generateThreeAddressCode(stBinop_Op1(t),0);
            generateThreeAddressCode(stBinop_Op2(t),0);
            tmp = newTmpVar(t->etype);
            newExprInstr(OpPlus, stBinop_Op1(t)->val.strefNode.stptr, stBinop_Op2(t)->val.strefNode.stptr, tmp);
            t->val.strefNode.stptr = tmp;
            break;
        case BinaryMinus:
            generateThreeAddressCode(stBinop_Op1(t),0);
            generateThreeAddressCode(stBinop_Op2(t),0);
            tmp = newTmpVar(t->etype);
            newExprInstr(OpBinaryMinus, stBinop_Op1(t)->val.strefNode.stptr, stBinop_Op2(t)->val.strefNode.stptr, tmp);
            t->val.strefNode.stptr = tmp;
            break;
        case Mult:
            generateThreeAddressCode(stBinop_Op1(t),0);
            generateThreeAddressCode(stBinop_Op2(t),0);
            tmp = newTmpVar(t->etype);
            newExprInstr(OpMult, stBinop_Op1(t)->val.strefNode.stptr, stBinop_Op2(t)->val.strefNode.stptr, tmp);
            t->val.strefNode.stptr = tmp;
            break;
        case Div:
            generateThreeAddressCode(stBinop_Op1(t),0);
            generateThreeAddressCode(stBinop_Op2(t),0);
            tmp = newTmpVar(t->etype);
            newExprInstr(OpDiv, stBinop_Op1(t)->val.strefNode.stptr, stBinop_Op2(t)->val.strefNode.stptr, tmp);
            t->val.strefNode.stptr = tmp;
            break;
        case Equals:
            generateThreeAddressCode(stBinop_Op1(t),0);
            generateThreeAddressCode(stBinop_Op2(t),0);
            newTrueConditionInstr(OpEq, stBinop_Op1(t)->val.strefNode.stptr, stBinop_Op2(t)->val.strefNode.stptr,
                                  trueLabel);
            newGotoInstr(falseLabel);

            break;
        case Neq:
            generateThreeAddressCode(stBinop_Op1(t),0);
            generateThreeAddressCode(stBinop_Op2(t),0);
            newTrueConditionInstr(OpNeq, stBinop_Op1(t)->val.strefNode.stptr, stBinop_Op2(t)->val.strefNode.stptr,
                                  trueLabel);
            newGotoInstr(falseLabel);

            break;
        case Leq:
            generateThreeAddressCode(stBinop_Op1(t),0);
            generateThreeAddressCode(stBinop_Op2(t),0);
            newTrueConditionInstr(OpLeq, stBinop_Op1(t)->val.strefNode.stptr, stBinop_Op2(t)->val.strefNode.stptr,
                                  trueLabel);
            newGotoInstr(falseLabel);

            break;
        case Lt:
            generateThreeAddressCode(stBinop_Op1(t),0);
            generateThreeAddressCode(stBinop_Op2(t),0);
            newTrueConditionInstr(OpLt, stBinop_Op1(t)->val.strefNode.stptr, stBinop_Op2(t)->val.strefNode.stptr,
                                  trueLabel);
            newGotoInstr(falseLabel);

            break;
        case Geq:
            generateThreeAddressCode(stBinop_Op1(t),0);
            generateThreeAddressCode(stBinop_Op2(t),0);
            newTrueConditionInstr(OpGeq, stBinop_Op1(t)->val.strefNode.stptr, stBinop_Op2(t)->val.strefNode.stptr,
                                  trueLabel);
            newGotoInstr(falseLabel);

            break;
        case Gt:
            generateThreeAddressCode(stBinop_Op1(t),0);
            generateThreeAddressCode(stBinop_Op2(t),0);
            newTrueConditionInstr(OpGt, stBinop_Op1(t)->val.strefNode.stptr, stBinop_Op2(t)->val.strefNode.stptr,
                                  trueLabel);
            newGotoInstr(falseLabel);

            break;
        case LogicalAnd: {
            char *middleLabel = newLabelName();

            char *tmpLabel = trueLabel;
            trueLabel = middleLabel;
            generateThreeAddressCode(stBinop_Op1(t),0);
            newGotoInstr(falseLabel);

            trueLabel = tmpLabel;
            newLabelInstr(middleLabel);
            generateThreeAddressCode(stBinop_Op2(t),0);
            newGotoInstr(falseLabel);

            break;
        }
        case LogicalOr: {
            char *middleLabel = newLabelName();

            char *tmpLabel = falseLabel;
            falseLabel = middleLabel;
            generateThreeAddressCode(stBinop_Op1(t),0);
            newGotoInstr(falseLabel);

            falseLabel = tmpLabel;
            newLabelInstr(middleLabel);
            generateThreeAddressCode(stBinop_Op2(t),0);
            newGotoInstr(falseLabel);

            break;
        }
        case LogicalNot: {
            char *tmpLabel = trueLabel;
            trueLabel = falseLabel;
            falseLabel = tmpLabel;
            generateThreeAddressCode(stUnop_Op(t),0);
            newGotoInstr(falseLabel);

            break;
        }
        case For: {
            char *bodyLabel = newLabelName();
            char *conditionLabel = newLabelName();
            char *endLabel = newLabelName();

            // init
            generateThreeAddressCode(stFor_Init(t),0);

            newGotoInstr(conditionLabel);

            // body branch
            newLabelInstr(bodyLabel);
            generateThreeAddressCode(stFor_Body(t),0);
            generateThreeAddressCode(stFor_Update(t),0);

            // condition branch
            newLabelInstr(conditionLabel);
            trueLabel = bodyLabel;
            falseLabel = endLabel;
            if (stFor_Test(t)) {
                generateThreeAddressCode(stFor_Test(t),0);
                newGotoInstr(falseLabel);
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
            generateThreeAddressCode(stWhile_Body(t),0);
            newGotoInstr(conditionLabel);

            // condition branch
            newLabelInstr(conditionLabel);
            trueLabel = bodyLabel;
            falseLabel = endLabel;
            generateThreeAddressCode(stWhile_Test(t),0);
            newGotoInstr(falseLabel);

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
                generateThreeAddressCode(stIf_Test(t),0);
                newGotoInstr(falseLabel);

                newLabelInstr(thenLabel);
                generateThreeAddressCode(stIf_Then(t),0);
                newGotoInstr(endLabel);

                newLabelInstr(elseLabel);
                generateThreeAddressCode(stIf_Else(t),0);
                newGotoInstr(endLabel);

            } else {
                // no else
                falseLabel = endLabel;
                trueLabel = thenLabel;
                generateThreeAddressCode(stIf_Test(t),0);
                newGotoInstr(falseLabel);

                newLabelInstr(thenLabel);
                generateThreeAddressCode(stIf_Then(t),0);
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
            } else if (stptr->type == t_Array) {
                if (stptr->elt_type == t_Char) {
                    printf("_%s:.space %d \n", stptr->name, 1 * stptr->num_elts);
                } else if (stptr->elt_type == t_Int) {
                    printf("_%s:.space %d \n", stptr->name, 4 * stptr->num_elts);
                }
                printf(".align 2 \n");
            }
        }
    }
    symtabnode *current = stringList.head;
    while (current) {
        printf("_%s: .asciiz \"%s\" \n", current->name, current->string);
        printf(".align 2 \n");
        current = current->next;
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

    for(symtabnode *stptr = fn_name->formals;stptr != NULL; stptr = stptr->next){
        stptr->offset = paramOffset;
        paramOffset += 4;
    }

    for (int i = 0; i < HASHTBLSZ; i++) {
        for (symtabnode *stptr = localTab[i]; stptr != NULL; stptr = stptr->next) {
            if (!stptr->formal) {
                if (stptr->type == t_Int) {
                    varSpace += 4;
                    stptr->offset = -varSpace;
                } else if (stptr->type == t_Char) {
                    // varSpace += 1;
                    varSpace += 4;
                    stptr->offset = -varSpace;
                } else if (stptr->type == t_Array) {
                    // Need review
                    varSpace += (stptr->num_elts+1) * 4;
                    stptr->offset = -varSpace;
                }
            }else{
                stptr->offset = -1;
                for(symtabnode *s = fn_name->formals;s != NULL; s = s->next){
                    if (!strcmp(s->name,stptr->name)){
                        stptr->offset = s->offset;
                    }
                }
            }
        }
    }

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

                if (left->type == t_Array) {
                    read(curr_instruction->value.assign.right, "$t0");
                    readString(curr_instruction->value.assign.left, "$t1",'w');
                    printf("  s%c $t0, 0($t1) \n", bOrW(curr_instruction->value.assign.left->elt_type));
                } else {
                    read(right, "$t0");
                    save(left, "$t0");
                }
                break;
            case OpParam: {
                printf("\n");
                printf("  # OpParam \n");

                if(curr_instruction->value.param->formal){
                    readString(curr_instruction->value.param, "$t0",'w');
                }else{
                    read(curr_instruction->value.param, "$t0");
                }
                printf("  la $sp, -4($sp)  \n");
                printf("  sw $t0, 0($sp)   \n");
                break;
            }

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
                if (curr_instruction->value.returnVar) {
                    read(curr_instruction->value.returnVar, "$v0");
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
                break;

            case OpArray:
                printf("\n");
                printf("  # OpArray \n");
                if(curr_instruction->value.array.address->elt_type == t_Char){
                    readString(curr_instruction->value.array.arr,"$t0",'b');
                }else if(curr_instruction->value.array.address->elt_type == t_Int){
                    readString(curr_instruction->value.array.arr,"$t0",'w');
                }
//                read(curr_instruction->value.array.arr,"$t0");
                // Load address of the first position of the array into $t1
                if (curr_instruction->value.array.subscript->formal) {
                    readString(curr_instruction->value.array.subscript, "$t1",'w');
                } else {
                    readString(curr_instruction->value.array.subscript, "$t1",'a');
                }
                // Find the correct memory address of the index
                if (curr_instruction->value.array.subscript->elt_type == t_Int) {
                    printf("  sll $t0, $t0, 2  \n");
                }
                printf("  add $t1, $t0, $t1 \n");
                saveString(curr_instruction->value.array.address, "$t1",'w');
                break;

            case OpDeref:
                printf("\n");
                printf("  # OpDeref \n");
                readString(curr_instruction->value.assign.right, "$t0",'w');
                printf("  l%c $t1, 0($t0) \n", bOrW(curr_instruction->value.assign.left->type));
                save(curr_instruction->value.assign.left, "$t1");
                break;
            default:
                break;
        }
        curr_instruction = curr_instruction->next;
    }
}


