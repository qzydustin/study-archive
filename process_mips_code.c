#include <stdarg.h>
#include "global.h"
#include "protos.h"
#include "syntax-tree.h"
#include "three-address-code.h"

#define HASHTBLSZ 256
int flag = 0;
extern symtabnode **globalTab;
extern symtabnode **localTab;

extern instr *instrHead;
extern instr *instrTail;
extern instr *globalTail;

int isPrintChar = 0;
symtabnode *newTempVar(int type)
{
    char name[10] = "_tmp";
    sprintf(name, "_tmp%d", tempCounter);
    tempCounter++;
    symtabnode *tmp = SymTabInsert(name, Local);
    tmp->type = type;
    return tmp;
}

void generateThreeAddressCode(tnode *t)
{
    symtabnode *tmp;

    if (!t)
    {
        return;
    }

    switch (t->ntype)
    {
    case Assg:
        generateThreeAddressCode(stAssg_Lhs(t));
        generateThreeAddressCode(stAssg_Rhs(t));
        newInstr(OpAssign, stAssg_Lhs(t)->val.strefNode.stptr, stAssg_Rhs(t)->val.strefNode.stptr, NULL);
        break;

    case Var:
        t->val.strefNode.stptr = stVar(t);
        if (t->val.strefNode.stptr->scope == Global)
        {
            int isNewGlobal = true;
            instr *current = instrHead;
            while (current)
            {
                if (!strcmp(current->value.global.name, t->val.strefNode.stptr->name))
                {
                    // branch: if the Global Var exists in instrList
                    isNewGlobal = false;
                    break;
                }
                if (current == globalTail)
                {
                    break;
                }
                current = current->next;
            }
            if (isNewGlobal)
            {
                newGlobalInstr(OpGlobal, t->val.strefNode.stptr->name, t->val.strefNode.stptr->type);
            }
        }
        break;

    case Intcon:
        tmp = newTempVar(t->etype);
        newConstInstr(OpConstInt, tmp, t->val.iconst);
        t->val.strefNode.stptr = tmp;

        break;

    case Charcon:
        tmp = newTempVar(t->etype);
        newConstInstr(OpConstChar, tmp, t->val.iconst);
        t->val.strefNode.stptr = tmp;
        break;
    case FunCall:
        // Expand the parameters
        generateThreeAddressCode(stFunCall_Args(t));
        // Create PARAM instructions
        instr *params_instructions = NULL;
        instr *instruction;
        int NumParams = 0;
        for (tnode *param = stFunCall_Args(t); param != NULL; param = stList_Rest(param))
        {
            instruction = newParamInstr(OpParam, stList_Head(param)->val.strefNode.stptr);
            // Actuals from the right to the left
            instruction->next = params_instructions;
            params_instructions = instruction;
            NumParams++;
        }
        appendInstructions(params_instructions);

        // newInstr(OpCall, NULL, stFunCall_Fun(t), NULL);
        newCallInstr(stFunCall_Fun(t), NumParams);

        break;

    case Return:
        newInstr(OpReturn, NULL, NULL, NULL);
        break;

    case STnodeList:
        for (tnode *tntmp0 = t; tntmp0 != NULL; tntmp0 = stList_Rest(tntmp0))
        {
            generateThreeAddressCode(stList_Head(tntmp0));
        }
        break;

    default:
        break;
    }
}

void generateMipsCode(symtabnode *fn_name, instr *instrHead)
{
    if (flag == 0)
    {
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
        printf("_printCharln:       \n");
        printf("  li $v0, 1         \n");
        printf("  lb $a0, 0($sp)    \n");
        printf("  syscall           \n");
        printf("  li $v0, 4         \n");
        printf("  la $a0, _nl       \n");
        printf("  syscall           \n");
        printf("  jr $ra            \n");
        
        printf("main: j _main       \n");
        flag = 1;
    }
    int varSpace = 0;
    int paramOffset = -8;
    for (int i = 0; i < HASHTBLSZ; i++)
    {
        symtabnode *node = localTab[i];
        if (node && !node->formal)
        {
            if (node->type == t_Int)
            {
                varSpace += 4;
            }
            else if (node->type == t_Char)
            {
                // varSpace += 1;
                varSpace += 4;
            }
            node->offset = varSpace;
        }
        if (node && node->formal)
        {
            node->offset = paramOffset;
            paramOffset -=4;
        }

    }

    char loadInst[3];
    char saveInst[3];
    int num_formals;

    instr *curr_instruction = instrHead;

    while (curr_instruction)
    {
        switch (curr_instruction->op)
        {
        case OpGlobal:
        {
            printf("\n.data \n");
            if (curr_instruction->value.global.type == t_Int)
            {
                printf("%s:.space 4 \n", curr_instruction->value.global.name);
            }
            else if (curr_instruction->value.global.type == t_Char)
            {
                printf("%s:.space 1 \n", curr_instruction->value.global.name);
                printf(".align 2 \n");
            }
            break;
        }
        case OpEnter:
            printf("\n.text \n");
            printf("_%s:              \n", curr_instruction->value.opMember.src1->name);
            printf("  la $sp, -8($sp) # allocate space for old $fp and $ra\n");
            printf("  sw $fp, 4($sp)  # save old $fp\n");
            printf("  sw $ra, 0($sp)  # save return address\n");
            printf("  la $fp, 0($sp)  # set up frame pointer\n");
            printf("  la $sp, %d($sp) # allocate stack frame\n", -varSpace);
            break;

        case OpConstInt:
            printf("\n");
            printf("  # OpConstInt \n");
            if (curr_instruction->value.constant >> 16 == 0)
            {
                printf("  li $t0, %d \n", curr_instruction->value.constant);
            }
            else
            {
                printf("  lui $t0, %d \n", curr_instruction->value.constant >> 16);
                printf("  ori $t0, %d \n", curr_instruction->value.constant & 0xffff);
            }
            printf("  sw $t0, %d($fp) \n", -curr_instruction->dest->offset);
            break;

        case OpConstChar:
            printf("\n");
            printf("  # OpConstChar \n");
            printf("  li $t0, %d      \n", curr_instruction->value.constant);
            printf("  sb $t0, %d($fp) \n", -curr_instruction->dest->offset);
            break;

        case OpAssign:
            printf("\n");
            printf("  # OpAssign \n");

            if (curr_instruction->dest->type == t_Char)
            {
                strcpy(loadInst, "lb");
                strcpy(saveInst, "sb");
            }
            else if (curr_instruction->dest->type == t_Int)
            {
                strcpy(loadInst, "lw");
                strcpy(saveInst, "sw");
            }

            if (curr_instruction->value.opMember.src1->scope == Global)
            {
                printf("  %s $t0, %s \n", loadInst, curr_instruction->value.opMember.src1->name);
            }
            else
            {
                printf("  %s $t0, %d($fp) \n", loadInst, -curr_instruction->value.opMember.src1->offset);
            }

            if (curr_instruction->dest->scope == Global)
            {
                printf("  %s $t0, %s \n", saveInst, curr_instruction->dest->name);
            }
            else
            {
                printf("  %s $t0, %d($fp) \n", saveInst, -curr_instruction->dest->offset);
            }

            break;
        case OpParam:
            printf("\n");
            printf("  # OpParam \n");

            if (curr_instruction->value.opMember.src1->type == t_Char)
            {
                strcpy(loadInst, "lb");
                strcpy(saveInst, "sb");
                isPrintChar = 1;
            }
            else if (curr_instruction->value.opMember.src1->type == t_Int)
            {
                strcpy(loadInst, "lw");
                strcpy(saveInst, "sw");
                isPrintChar = 0;
            }

            if (curr_instruction->value.opMember.src1->scope == Global)
            {
                printf("  %s $t0, %s \n", loadInst, curr_instruction->value.opMember.src1->name);
            }
            else
            {
                printf("  %s $t0, %d($fp) \n", loadInst, -curr_instruction->value.opMember.src1->offset);
            }
            printf("  la $sp, -4($sp)  \n");
            printf("  %s $t0, 0($sp)   \n", saveInst);
            break;
        case OpCall:
            

            printf("\n");
            printf("  # OpCall       \n");

            if(!strcmp(curr_instruction->value.call.callee->name,"println")){
                if(isPrintChar){
                    printf("  jal _%s         \n", "printCharln");
                }else{
                    printf("  jal _%s         \n", "println");
                }
            }else{
                printf("  jal _%s         \n", curr_instruction->value.call.callee->name);
            }
            printf("  la $sp, %d($sp) \n", 4 * curr_instruction->value.call.numOfParams);
            break;

        case OpReturn:
        {
            printf("\n");
            printf("  # OpReturn \n");
            printf("  la $sp, 0($fp) # deallocate locals \n");
            printf("  lw $ra, 0($sp) # restore return address \n");
            printf("  lw $fp, 4($sp) # restore frame pointer \n");
            printf("  la $sp, 8($sp) # restore stack pointer \n");
            printf("  jr $ra         # return\n");
            break;
        }

        default:
            break;
        }
        curr_instruction = curr_instruction->next;
    }
}
