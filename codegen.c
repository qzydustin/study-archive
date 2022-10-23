/*
 * File: codegen.c
 * Author: Saumya Debray
 * Purpose: Three-address and final code generation for C--
 */

#include "global.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "protos.h"
#include "syntax-tree.h"
#include "codegen.h"
#include "util.h"

extern int oLocalFlag;
extern int oGlobalFlag;


extern symtabnode *symbol_table[];

/*
 * curr_fn_symtbl -- a pointer to the global symbol table entry of the current
 * function being processed.
 */
static symtabnode *curr_fn_symtbl = NULL;

/*******************************************************************************
 *                                                                             *
 *                                   PROTOTYPES                                *
 *                                                                             *
 *******************************************************************************/

static void gen_3addr_code(tnode *ast);

static void gen_mips_code(tnode *ast);

static void gen_3addr_code_funcbody(tnode *ast);

static void gen_3addr_code_funcall(tnode *ast);

static void gen_3addr_code_if(tnode *ast);

static void gen_3addr_code_while(tnode *ast);

static void gen_3addr_code_for(tnode *ast);

static void gen_3addr_code_assg(tnode *ast);

static void gen_3addr_code_return(tnode *ast);

static void gen_3addr_code_astlist(tnode *ast);

static void gen_3addr_code_id(tnode *ast);

static void gen_3addr_code_intconst(tnode *ast);

static void gen_3addr_code_stringcon(tnode *ast);

static void gen_3addr_code_arraysub(tnode *ast);

static void gen_3addr_code_arithop(tnode *ast);

static void gen_3addr_code_boolexp(tnode *ast, Quad *true_lbl, Quad *false_lbl);

static Op ast_binop(SyntaxNodeType ntype);

static void gen_mips_move(Quad *qptr);

static void gen_mips_arith(Quad *qptr);

static void gen_mips_goto(Quad *qptr);

static void gen_mips_if(Quad *qptr);

static void gen_mips_label(Quad *qptr);

static void gen_mips_enter(Quad *qptr);

static void gen_mips_leave(Quad *qptr);

static void gen_mips_param(Quad *qptr);

static void gen_mips_call(Quad *qptr);

static void gen_mips_ret(Quad *qptr);

static void gen_mips_retrieve(Quad *qptr);

static void gen_mips_load(Operand *src, char *reg_prefix, int dstreg);

static void gen_mips_store(Operand *dst, char *reg_prefix, int srcreg);

static void print_fixed_MIPS_code();

static void print_3addr_ins_no_nl(Quad *ins);

static void print_string_table();

/*
 * mk_intcon_op(numval) -- create an operand with optype INTEGER and value numval.
 */
static Operand *mk_intcon_op(int numval);

/*
 * mk_stptr_op(stptr) -- create an operand with optype SYMTBL_PTR and value stptr.
 */
static Operand *mk_stptr_op(symtabnode *stptr);

/*
 * mk_insptr_op(insptr) -- create an operand with optype INS_PTR and value insptr.
 */
static Operand *mk_insptr_op(Quad *insptr);

/*
 * mk_addr_op(insptr) -- create an operand with optype ADDR and value stptr.
 */
static Operand *mk_addr_op(symtabnode *stptr);

/*
 * mk_deref_op(insptr) -- create an operand with optype DEREF and value stptr.
 */
static Operand *mk_deref_op(symtabnode *stptr);

/*
 * mk_expr_op(sptr) -- If sptr has type t_Char or t_Int, creates and returns a
 * SYMTBL_PTR operand.  If sptr has type t_Addr, creates and returns a DEREF op.
 */
static Operand *mk_expr_op(symtabnode *stptr);

/*
 * If ast is an array expression, gen_3addr_code will generate code to compute
 * its address.  deref_lvalue() adds a dereference instruction at the end.
 */
static void deref_lvalue(tnode *ast);

/*
 * new_instr(op, src1, src2, dst) -- create a new three-address instruction
 * (i.e., a Quad data structure), set its fields to the arguments given, and
 * return the result.
 */
static Quad *new_instr(Op op, Operand *src1, Operand *src2, Operand *dst);

static symtabnode *new_temp(int type);

static Quad *new_label();

// optimize
//TODO
static void gen_optimize(tnode *ast);

static void peephole_optimize(Quad *fn_head);

static void copy_propagate(Quad *fn_head);

static int operands_compare(Operand *a, Operand *b);


/*******************************************************************************
 *                                                                             *
 *                                STRING TABLE                                 #
 *                                                                             *
 *******************************************************************************/

typedef struct strtab_node {
    char *str;
    char *str_lbl;   /* the label used to refer to this string */
    struct strtab_node *next;
} StrTabNode;

static int str_lbl_ctr = 0;    /* counter for string labels */
static StrTabNode *string_tbl = NULL, *str_tmp;

/*******************************************************************************
 *                                                                             *
 *                           TOP-LEVEL CODE GENERATION                         *
 *                                                                             *
 *******************************************************************************/

/*
 * gen_code(ast, sptr) -- given the AST ast and symbol table pointer sptr for a
 * function definition, generate 3-address code for it, then use the generated
 * 3-address code to generate MIPS asm code that is written out to stdout.
 */
void gen_code(tnode *ast, symtabnode *sptr) {
    printf("# oLocalFlag = %d\n", oLocalFlag);
    printf("# oGlobalFlag = %d\n", oGlobalFlag);
    curr_fn_symtbl = sptr;
    gen_3addr_code_funcbody(ast);
    gen_optimize(ast);
    gen_mips_code(ast);
}

/*
 * gen_code_global() -- generates global code sequences that need to be generated
 * only once.  This consists of:
 *    -- .space declarations for global variables
 *    -- code for the println() function
 *    -- the jump from main to _main
 */
void gen_code_global() {
    print_globals();
    print_string_table();
    print_fixed_MIPS_code();
}


/*******************************************************************************
 *                                                                             *
 *                         THREE-ADDRESS CODE GENERATION                       *
 *                                                                             *
 *******************************************************************************/

/*
 * gen_3addr_code(ast) -- given a pointer ast to an AST node, traverse the AST
 * and construct a list of three-address instructions at each node of the AST.
 */
static void gen_3addr_code(tnode *ast) {
    if (ast == NULL) {
        return;
    }

    switch (ast->ntype) {
        case FunCall:   /* function call */
            gen_3addr_code_funcall(ast);
            break;

        case If:          /* if statement */
            gen_3addr_code_if(ast);
            break;

        case While:       /* while statement */
            gen_3addr_code_while(ast);
            break;

        case For:
            gen_3addr_code_for(ast);
            break;

        case Assg:        /* assignment statement */
            gen_3addr_code_assg(ast);
            break;

        case Return:      /* return statement */
            gen_3addr_code_return(ast);
            break;

        case STnodeList:  /* list of AST nodes */
            gen_3addr_code_astlist(ast);
            break;

        case Var:         /* identifier */
            gen_3addr_code_id(ast);
            break;

        case ArraySubscript:  /* array subscript */
            gen_3addr_code_arraysub(ast);
            break;

        case Intcon:    /* integer constant */
        case Charcon:   /* character constant */
            gen_3addr_code_intconst(ast);
            break;

        case Stringcon:
            gen_3addr_code_stringcon(ast);
            break;

        case Plus:        /* + */
        case BinaryMinus: /* - (binary) */
        case Mult:        /* * */
        case Div:         /* / */
        case UnaryMinus:  /* - (unary) */
            gen_3addr_code_arithop(ast);
            break;

        case Equals:      /* == */
        case Neq:         /* != */
        case Leq:         /* <= */
        case Lt:          /* < */
        case Geq:         /* >= */
        case Gt:          /* > */
        case LogicalAnd:  /* && */
        case LogicalOr:   /* || */
        case LogicalNot:  /* || */
            /*
             * Relational and Boolean operators should be handled while processing
             * IF and WHILE statements, and so should not be encountered here.
             */
            assert(0);
            break;

        default:
            fprintf(stderr, "Unrecognized/unhandled AST node type: %d\n", ast->ntype);
            exit(1);
    }
}

static void gen_3addr_code_funcbody(tnode *ast) {
    assert(ast != NULL);
    /*
     * generate code for the body
     */
    gen_3addr_code(ast);
    /*
     * create ENTER, LEAVE, and RETURN instructions and stitch them
     * around the code for the body.
     */
    Operand *stptr_op = mk_stptr_op(curr_fn_symtbl);
    Quad *enter_ins = new_instr(ENTER, stptr_op, NULL, NULL);
    Quad *leave_ins = new_instr(LEAVE, stptr_op, NULL, NULL);
    Quad *ret_ins = new_instr(RET, NULL, NULL, NULL);

    if (ast == NULL) {
        enter_ins->next = leave_ins;
    } else {
        enter_ins->next = ast->code_hd;
        ast->code_tl->next = leave_ins;
    }
    leave_ins->next = ret_ins;
    /*
     * set the code pointer(s) for the function definition
     */
    ast->code_hd = enter_ins;
    ast->code_tl = ret_ins;
}


static void gen_3addr_code_funcall(tnode *ast) {
    assert(ast != NULL && ast->ntype == FunCall);
    Quad *parms_hd = NULL, *parms_tl = NULL, *parm_instr, *call_instr, *retrieve_instr;
    tnode *atmp;
    symtabnode *retval;
    symtabnode *callee = stFunCall_Fun(ast);
    tnode *args = stFunCall_Args(ast);

    gen_3addr_code(args);    /* code to evaluate the arguments */
    /*
     * iterate down the expression list and create the param instructions
     */
    for (atmp = stFunCall_Args(ast); atmp != NULL; atmp = stList_Rest(atmp)) {
        parm_instr = new_instr(PARAM, mk_expr_op(stList_Head(atmp)->place), NULL, NULL);
        if (parms_hd == NULL) {
            parms_hd = parms_tl = parm_instr;
        } else {
            parm_instr->next = parms_hd;
            parms_hd = parm_instr;
        }
    }

    call_instr = new_instr(CALL,
                           mk_stptr_op(callee),
                           mk_intcon_op(callee->nargs),
                           NULL);

    if (callee->ret_type == t_None) {  /* void */
        retrieve_instr = new_instr(NOP, NULL, NULL, NULL);
    } else {
        retval = new_temp(t_Int);
        retrieve_instr = new_instr(RETRIEVE, mk_stptr_op(retval), NULL, NULL);
        ast->place = retval;
    }

    if (args != NULL) {
        if (args->code_hd != NULL) {
            ast->code_hd = args->code_hd;
            args->code_tl->next = parms_hd;
        } else {
            ast->code_hd = parms_hd;
        }
        parms_tl->next = call_instr;
    } else {
        ast->code_hd = call_instr;
    }

    call_instr->next = retrieve_instr;
    ast->code_tl = retrieve_instr;
}


static void gen_3addr_code_if(tnode *ast) {
    assert(ast != NULL && ast->ntype == If);

    Quad *lbl_then = new_label();
    Quad *lbl_else = new_label();
    Quad *lbl_after = new_label();
    Quad *jmp_over = new_instr(GOTO, NULL, NULL, mk_insptr_op(lbl_after));

    gen_3addr_code_boolexp(Child0(ast), lbl_then, lbl_else);
    ast->code_hd = Child0(ast)->code_hd;

    Child0(ast)->code_tl->next = lbl_then;

    if (Child1(ast)) {
        gen_3addr_code(Child1(ast));
        lbl_then->next = Child1(ast)->code_hd;
        Child1(ast)->code_tl->next = jmp_over;
    } else {
        lbl_then->next = jmp_over;
    }

    jmp_over->next = lbl_else;

    if (Child2(ast)) {
        gen_3addr_code(Child2(ast));
        lbl_else->next = Child2(ast)->code_hd;
        Child2(ast)->code_tl->next = lbl_after;
    } else {
        lbl_else->next = lbl_after;
    }

    ast->code_tl = lbl_after;
}

static void gen_3addr_code_while(tnode *ast) {
    assert(ast != NULL && ast->ntype == While);

    Quad *lbl_top, *lbl_body, *lbl_after, *br_back;

    lbl_top = new_label();
    lbl_body = new_label();
    lbl_after = new_label();

    br_back = new_instr(GOTO, NULL, NULL, mk_insptr_op(lbl_top));

    gen_3addr_code_boolexp(Child0(ast), lbl_body, lbl_after);
    gen_3addr_code(Child1(ast));

    ast->code_hd = lbl_top;
    lbl_top->next = Child0(ast)->code_hd;
    Child0(ast)->code_tl->next = lbl_body;
    if (Child1(ast)->code_hd != NULL) {
        lbl_body->next = Child1(ast)->code_hd;
        Child1(ast)->code_tl->next = br_back;
    } else {
        lbl_body->next = br_back;
    }
    br_back->next = lbl_after;
    ast->code_tl = lbl_after;

}


static void gen_3addr_code_for(tnode *ast) {
    assert(ast != NULL && ast->ntype == For);

    tnode *for_init, *for_test, *for_upd, *for_body;

    Quad *lbl_top, *lbl_body, *lbl_after, *br_back;
    Quad *nop0, *nop1, *nop2, *nop3;
    Quad *ch0hd, *ch0tl, *ch1hd, *ch1tl, *ch2hd, *ch2tl, *ch3hd, *ch3tl;

    for_init = stFor_Init(ast);
    for_test = stFor_Test(ast);
    for_upd = stFor_Update(ast);
    for_body = stFor_Body(ast);

    lbl_top = new_label();
    lbl_body = new_label();
    lbl_after = new_label();

    br_back = new_instr(GOTO, NULL, NULL, mk_insptr_op(lbl_top));

    if (for_init != NULL) {
        gen_3addr_code(for_init);
        ch0hd = for_init->code_hd;
        ch0tl = for_init->code_tl;
    } else {
        nop0 = new_instr(NOP, NULL, NULL, NULL);
        ch0hd = ch0tl = nop0;
    }

    if (for_test != NULL) {
        gen_3addr_code_boolexp(for_test, lbl_body, lbl_after);
        ch1hd = for_test->code_hd;
        ch1tl = for_test->code_tl;
    } else {
        nop1 = new_instr(NOP, NULL, NULL, NULL);
        ch1hd = ch1tl = nop1;
    }

    if (for_upd != NULL) {
        gen_3addr_code(for_upd);
        ch2hd = for_upd->code_hd;
        ch2tl = for_upd->code_tl;
    } else {
        nop2 = new_instr(NOP, NULL, NULL, NULL);
        ch2hd = ch2tl = nop2;
    }

    if (for_body != NULL) {
        gen_3addr_code(for_body);
        ch3hd = for_body->code_hd;
        ch3tl = for_body->code_tl;
    } else {
        nop3 = new_instr(NOP, NULL, NULL, NULL);
        ch3hd = ch3tl = nop3;
    }

    ast->code_hd = ch0hd;
    ch0tl->next = lbl_top;

    lbl_top->next = ch1hd;
    ch1tl->next = lbl_body;

    lbl_body->next = ch3hd;
    ch3tl->next = ch2hd;

    ch2tl->next = br_back;

    br_back->next = lbl_after;
    ast->code_tl = lbl_after;

}


static void gen_3addr_code_assg(tnode *ast) {
    assert(ast != NULL && ast->ntype == Assg);

    gen_3addr_code(Child0(ast));    /* LHS of the assignment */
    gen_3addr_code(Child1(ast));    /* RHS of the assignment */

    Quad *qptr = new_instr(MOVE,
                           mk_expr_op(Child1(ast)->place),   /* src1 */
                           NULL,                             /* src2 */
                           mk_expr_op(Child0(ast)->place)    /* dst */
    );

    if (Child0(ast)->code_hd == NULL) {
        ast->code_hd = ast->code_tl = qptr;
    } else {
        ast->code_hd = Child0(ast)->code_hd;
        Child0(ast)->code_tl->next = Child1(ast)->code_hd;
        Child1(ast)->code_tl->next = qptr;
        ast->code_tl = qptr;
    }
}

static void gen_3addr_code_return(tnode *ast) {
    assert(ast != NULL && ast->ntype == Return);

    Quad *qptr_leave = new_instr(LEAVE, mk_stptr_op(curr_fn_symtbl), NULL, NULL);
    Quad *qptr_ret = new_instr(RET, NULL, NULL, NULL);
    Quad *cvt_type;
    symtabnode *ret_val;

    qptr_leave->next = qptr_ret;

    if (Child0(ast)) {
        gen_3addr_code(Child0(ast));
        /*
         * add type conversion code if necessary
         */
        if (curr_fn_symtbl->ret_type != Child0(ast)->place->type) {
            ret_val = new_temp(curr_fn_symtbl->ret_type);
            cvt_type = new_instr(MOVE,
                                 mk_stptr_op(Child0(ast)->place),  /* src1 */
                                 NULL,                             /* src2 */
                                 mk_stptr_op(ret_val)              /* dst */
            );
            qptr_ret->src1 = mk_stptr_op(ret_val);
            ast->code_hd = Child0(ast)->code_hd;
            Child0(ast)->code_tl->next = cvt_type;
            cvt_type->next = qptr_leave;
        } else {
            qptr_ret->src1 = mk_stptr_op(Child0(ast)->place);
            ast->code_hd = Child0(ast)->code_hd;
            Child0(ast)->code_tl->next = qptr_leave;
        }
        ast->code_tl = qptr_ret;
    } else {
        ast->code_hd = qptr_leave;
        ast->code_tl = qptr_ret;
    }
}

static void gen_3addr_code_astlist(tnode *ast) {
    tnode *ast0, *ast1, *ast2;

    assert(ast != NULL && ast->ntype == STnodeList);

    /*
     * first pass: iterate over the AST list generating 3-address
     * code for each element of the list.
     */
    for (ast0 = ast; ast0 != NULL; ast0 = Child1(ast0)) {
        gen_3addr_code(Child0(ast0));
    }

    ast->code_hd = Child0(ast)->code_hd;

    /*
     * second pass: iterate over the AST list and chain together
     * their 3-addr code lists.
     */
    for (ast0 = ast; ast0 != NULL; ast0 = Child1(ast0)) {
        ast1 = Child0(ast0);
        if (Child1(ast0) == NULL) {    /* last node in the AST list */
            ast->code_tl = ast1->code_tl;
        } else {
            ast2 = Child0(Child1(ast0));
            ast1->code_tl->next = ast2->code_hd;
        }
    }
}

static void gen_3addr_code_id(tnode *ast) {
    assert(ast != NULL && ast->ntype == Var);

    ast->code_hd = ast->code_tl = new_instr(NOP, NULL, NULL, NULL);
    ast->place = stVar(ast);
}

static void gen_3addr_code_intconst(tnode *ast) {
    assert(ast != NULL && (ast->ntype == Intcon || ast->ntype == Charcon));

    symtabnode *lhs = new_temp(t_Int);
    Quad *qptr = new_instr(MOVE,
                           mk_intcon_op(ConstVal(ast)),    /* src1 */
                           NULL,                           /* src2 */
                           mk_stptr_op(lhs)                /* dst */
    );
    ast->place = lhs;
    ast->code_hd = ast->code_tl = qptr;

}

static void gen_3addr_code_stringcon(tnode *ast) {
    char strbuf[60], *strptr;
    symtabnode *sptr;
    Quad *instr;

    /* add the string to the string table */
    str_tmp = zalloc(sizeof(StrTabNode));
    str_tmp->str = stStringcon(ast);
    snprintf(strbuf, 60, "__Str%d", str_lbl_ctr++);
    strptr = strdup(strbuf);
    str_tmp->str_lbl = strptr;
    str_tmp->next = string_tbl;
    string_tbl = str_tmp;
    /*
     * add an entry to the global symbol table for the string with name =
     * the string label (but with the leading underscore removed, since that gets
     * added on when the label is referenced) and type = array of char.
     * Set the skip_print field of the symbol table entry since the string label
     * will be printed when the string table is dumped.
     */
    sptr = SymTabInsert(strptr + 1, Global);
    sptr->type = t_Array;
    sptr->elt_type = t_Char;
    sptr->num_elts = strlen(1 + str_tmp->str_lbl);  /* add 1 for the trailing NUL */
    sptr->skip_print = 1;

    instr = new_instr(NOP, NULL, NULL, NULL);
    ast->code_hd = ast->code_tl = instr;

    ast->place = sptr;
}


static void gen_3addr_code_arraysub(tnode *ast) {
    assert(ast != NULL && ast->ntype == ArraySubscript);

    symtabnode *disp_loc, *addr_loc;
    Quad *qptr_addr, *qptr_mul, *qptr_add;

    tnode *idx = stArraySubscript_Subscript(ast);
    symtabnode *sptr = stArraySubscript_Array(ast);
    int width = sptr->elt_type == t_Char ? 1 : 4;

    addr_loc = new_temp(t_Addr);
    addr_loc->elt_type = sptr->elt_type;

    disp_loc = new_temp(t_Int);

    gen_3addr_code(idx);
    if (idx->ntype == ArraySubscript) {
        deref_lvalue(idx);
    }
    /*
     * Since arrays are passed by reference, if sptr is a formal parameter then it
     * already contains the address of the array.
     */
    if (sptr->scope == Local && sptr->formal) {
        qptr_addr = new_instr(MOVE,
                              mk_stptr_op(sptr),
                              NULL,
                              mk_stptr_op(addr_loc)
        );
    } else {
        qptr_addr = new_instr(MOVE,
                              mk_addr_op(sptr),
                              NULL,
                              mk_stptr_op(addr_loc)
        );
    }

    qptr_mul = new_instr(MUL_OP,
                         mk_stptr_op(idx->place),
                         mk_intcon_op(width),
                         mk_stptr_op(disp_loc));
    qptr_add = new_instr(ADD_OP,
                         mk_stptr_op(addr_loc),
                         mk_stptr_op(disp_loc),
                         mk_stptr_op(addr_loc));

    /* ast->place: the location where the address of the array element will be computed */
    ast->place = addr_loc;
    ast->code_hd = idx->code_hd;
    idx->code_tl->next = qptr_addr;
    qptr_addr->next = qptr_mul;
    qptr_mul->next = qptr_add;
    ast->code_tl = qptr_add;
}

static void gen_3addr_code_arithop(tnode *ast) {
    Quad *arith_inst;
    Op op;
    Operand *src1, *src2;

    ast->place = new_temp(t_Int);
    gen_3addr_code(Child0(ast));
    if (Child0(ast)->ntype == ArraySubscript) {
        deref_lvalue(Child0(ast));
    }

    src1 = mk_stptr_op(Child0(ast)->place);

    if (ast->ntype == UnaryMinus) {
        op = UMINUS_OP;
        src2 = NULL;
    } else {
        op = ast_binop(ast->ntype);
        gen_3addr_code(Child1(ast));
        if (Child1(ast)->ntype == ArraySubscript) {
            deref_lvalue(Child1(ast));
        }

        src2 = mk_stptr_op(Child1(ast)->place);
    }

    arith_inst = new_instr(op, src1, src2, mk_stptr_op(ast->place));

    if (ast->ntype == UnaryMinus) {
        Child0(ast)->code_tl->next = arith_inst;
    } else {
        Child0(ast)->code_tl->next = Child1(ast)->code_hd;
        Child1(ast)->code_tl->next = arith_inst;
    }

    ast->code_hd = Child0(ast)->code_hd;
    ast->code_tl = arith_inst;
}

static void gen_3addr_code_boolexp(tnode *ast, Quad *true_lbl, Quad *false_lbl) {
    Quad *mid_lbl;
    Op op;

    assert(ast != NULL);

    switch (ast->ntype) {
        case Equals:
        case Neq:
        case Leq:
        case Lt:
        case Geq:
        case Gt:
            op = ast_binop(ast->ntype);

            gen_3addr_code(Child0(ast));
            if (Child0(ast)->ntype == ArraySubscript) {
                deref_lvalue(Child0(ast));
            }

            gen_3addr_code(Child1(ast));
            if (Child1(ast)->ntype == ArraySubscript) {
                deref_lvalue(Child1(ast));
            }

            Quad *ins1 = new_instr(op,
                                   mk_stptr_op(Child0(ast)->place),    /* src1 */
                                   mk_stptr_op(Child1(ast)->place),    /* src1 */
                                   mk_insptr_op(true_lbl)              /* dst */
            );
            Quad *ins2 = new_instr(GOTO, NULL, NULL, mk_insptr_op(false_lbl));

            if (Child0(ast)->code_hd != NULL) {
                ast->code_hd = Child0(ast)->code_hd;
                if (Child1(ast)->code_hd != NULL) {
                    Child0(ast)->code_tl->next = Child1(ast)->code_hd;
                    Child1(ast)->code_tl->next = ins1;
                } else {
                    Child0(ast)->code_tl->next = ins1;
                }
            } else {
                if (Child1(ast)->code_hd != NULL) {
                    ast->code_hd = Child1(ast)->code_hd;
                    Child1(ast)->code_tl->next = ins1;
                } else {
                    ast->code_hd = ins1;
                }
            }

            ins1->next = ins2;
            ast->code_tl = ins2;
            break;

        case LogicalAnd:
            mid_lbl = new_label();
            gen_3addr_code_boolexp(Child0(ast), mid_lbl, false_lbl);
            gen_3addr_code_boolexp(Child1(ast), true_lbl, false_lbl);
            ast->code_hd = Child0(ast)->code_hd;
            Child0(ast)->code_tl->next = mid_lbl;
            mid_lbl->next = Child1(ast)->code_hd;
            ast->code_tl = Child1(ast)->code_tl;
            break;

        case LogicalOr:
            mid_lbl = new_label();
            gen_3addr_code_boolexp(Child0(ast), true_lbl, mid_lbl);
            gen_3addr_code_boolexp(Child1(ast), true_lbl, false_lbl);
            ast->code_hd = Child0(ast)->code_hd;
            Child0(ast)->code_tl->next = mid_lbl;
            mid_lbl->next = Child1(ast)->code_hd;
            ast->code_tl = Child1(ast)->code_tl;
            break;

        case LogicalNot:
            gen_3addr_code_boolexp(Child0(ast), false_lbl, true_lbl);
            ast->code_hd = Child0(ast)->code_hd;
            ast->code_tl = Child0(ast)->code_tl;
            break;

        default:
            fprintf(stderr, "*** [%s] Unrecognized/unhandled AST nodetype %d\n",
                    __func__,
                    ast->ntype);
            exit(1);

    }
}


static Op ast_binop(SyntaxNodeType ntype) {
    switch (ntype) {
        case Equals:
            return IF_EQ;
        case Neq:
            return IF_NE;
        case Leq:
            return IF_LE;
        case Lt:
            return IF_LT;
        case Geq:
            return IF_GE;
        case Gt:
            return IF_GT;
        case Plus:
            return ADD_OP;
        case BinaryMinus:
            return SUB_OP;
        case Mult:
            return MUL_OP;
        case Div:
            return DIV_OP;
        case UnaryMinus:
            return UMINUS_OP;
        default:
            fprintf(stderr, "*** [%s] Unrecognized/unhandled AST nodetype %d\n",
                    __func__,
                    ntype);
            exit(1);
    }
}


/*******************************************************************************
 *                                                                             *
 *                             FINAL CODE GENERATION                           *
 *                                                                             *
 *******************************************************************************/

#define LABEL_PREFIX  "Lbl"

/*
 * LoadIns[] and StoreIns[] are indexed by the type of the location being accessed,
 * and give the MIPS instruction for accessing a location of that type.
 */
static char *LoadIns[3] = {"lb", "lw", "lw"};
static char *StoreIns[3] = {"sb", "sw", "sw"};

/*
 * gen_mips_code() -- takes a pointer to a function definition AST that has been
 * decorated with three-address instructions, and emits MIPS instructions for them.
 */
static void gen_mips_code(tnode *ast) {
    symtabnode *sptr = curr_fn_symtbl;
    Quad *qptr;

    assert(ast != NULL);

    sptr->offset = allocate_storage(sptr);

    printf(".text\n");
    dump_fn_info(curr_fn_symtbl);
    printf("_%s:\n", sptr->name);

    for (qptr = ast->code_hd; qptr != NULL; qptr = qptr->next) {
        printf("    # ");
        print_3addr_ins(qptr);

        switch (qptr->op) {
            case NOP:
                /* do nothing -- no code to generate */
                break;

            case MOVE:
                gen_mips_move(qptr);
                break;

            case GOTO:
                gen_mips_goto(qptr);
                break;

            case IF_EQ:
            case IF_NE:
            case IF_LE:
            case IF_LT:
            case IF_GE:
            case IF_GT:
                gen_mips_if(qptr);
                break;

            case ADD_OP:
            case SUB_OP:
            case MUL_OP:
            case DIV_OP:
            case UMINUS_OP:
                gen_mips_arith(qptr);
                break;

            case LABEL:
                gen_mips_label(qptr);
                break;

            case ENTER:
                gen_mips_enter(qptr);
                break;

            case LEAVE:
                gen_mips_leave(qptr);
                break;

            case PARAM:
                gen_mips_param(qptr);
                break;

            case CALL:
                gen_mips_call(qptr);
                break;

            case RET:
                gen_mips_ret(qptr);
                break;

            case RETRIEVE:
                gen_mips_retrieve(qptr);
                break;

            default:
                fprintf(stderr, "*** [%s] Unrecognized op: %d\n", __func__, qptr->op);
                exit(1);
        }
        printf("\n");
    }
    printf("# end function %s\n\n", sptr->name);

}


static void gen_mips_move(Quad *qptr) {
    assert(qptr->dst->optype == SYMTBL_PTR || qptr->dst->optype == DEREF);
    symtabnode *stptr = (qptr->dst->val).stptr;

    gen_mips_load(qptr->src1, "t", 0);

    switch (qptr->dst->optype) {
        case SYMTBL_PTR:
            switch (stptr->scope) {
                case Local:
                    printf("    %s $t0, %d($fp)\t# %s\n", StoreIns[stptr->type], stptr->offset, stptr->name);
                    break;

                case Global:
                    printf("    %s $t0, _%s\n", StoreIns[stptr->type], stptr->name);
                    break;

                default:
                    fprintf(stderr, "*** [%s] Unrecognized symbol table scope: %d\n",
                            __func__,
                            stptr->scope);
                    exit(1);
            }
            break;

        case DEREF:
            switch (stptr->scope) {
                case Local:
                    printf("    lw $t1, %d($fp)\t# %s\n", stptr->offset, stptr->name);
                    printf("    %s $t0, 0($t1)\t# deref(%s)\n", StoreIns[stptr->elt_type], stptr->name);
                    break;

                case Global:
                    printf("    lw $t1, _%s\n", stptr->name);
                    printf("    %s $t0, 0($t1)\n", StoreIns[stptr->elt_type]);
                    break;

                default:
                    fprintf(stderr, "*** [%s] Unrecognized symbol table scope: %d\n",
                            __func__,
                            stptr->scope);
                    exit(1);
            }
            break;

        default:
            fprintf(stderr, "*** [%s] Unrecognized dst optype: %d\n",
                    __func__,
                    qptr->dst->optype);
            exit(1);
    }
}

static void gen_mips_arith(Quad *qptr) {
    char *mips_op;

    switch (qptr->op) {
        case ADD_OP:
            mips_op = "add";
            break;
        case SUB_OP:
            mips_op = "sub";
            break;
        case MUL_OP:
            mips_op = "mul";
            break;
        case DIV_OP:
            mips_op = "div";
            break;
        case UMINUS_OP:
            mips_op = "neg";
            break;
        default:
            mips_op = "???";
    }

    gen_mips_load(qptr->src1, "t", 0);
    if (qptr->op == UMINUS_OP) {
        printf("    %s $t2, $t0\n", mips_op);
    } else {
        gen_mips_load(qptr->src2, "t", 1);
        printf("    %s $t2, $t0, $t1\n", mips_op);
    }

    gen_mips_store(qptr->dst, "t", 2);

}

static void gen_mips_goto(Quad *qptr) {
    int label_no;
    Quad *dstins = (qptr->dst->val).insptr;

    label_no = (dstins->src1->val).numval;
    printf("    j %s%d\n", LABEL_PREFIX, label_no);
}

static void gen_mips_if(Quad *qptr) {
    char *mips_op;
    int label_no;

    gen_mips_load(qptr->src1, "t", 0);
    gen_mips_load(qptr->src2, "t", 1);

    switch (qptr->op) {
        case IF_EQ:
            mips_op = "eq";
            break;
        case IF_NE:
            mips_op = "ne";
            break;
        case IF_LE:
            mips_op = "le";
            break;
        case IF_LT:
            mips_op = "lt";
            break;
        case IF_GE:
            mips_op = "ge";
            break;
        case IF_GT:
            mips_op = "gt";
            break;
        default:
            mips_op = "???";
    }

    Quad *dstins = (qptr->dst->val).insptr;
    label_no = (dstins->src1->val).numval;

    printf("    b%s $t%d, $t%d, %s%d\n", mips_op, 0, 1, LABEL_PREFIX, label_no);
}

static void gen_mips_label(Quad *qptr) {
    int label_no = (qptr->src1->val).numval;
    printf("%s%d:\n", LABEL_PREFIX, label_no);
}

static void gen_mips_enter(Quad *qptr) {
    symtabnode *sptr = qptr->src1->val.stptr;

    printf("    la $sp, -8($sp)    # allocate space for old $fp and $ra\n");
    printf("    sw $fp, 4($sp)     # save old $fp\n");
    printf("    sw $ra, 0($sp)     # save old $ra\n");
    printf("    la $fp, 0($sp)     # $fp := $sp\n");
    printf("    la $sp, -%d($sp)   # allocate stack frame\n", sptr->offset);
}

static void gen_mips_leave(Quad *qptr) {
    /* restore callee-saved registers -- ignore for CSC 453 */
}

static void gen_mips_param(Quad *qptr) {
    gen_mips_load(qptr->src1, "t", 0);
    printf("    la $sp, -4($sp)\n");
    printf("    sw $t0, 0($sp)\n");
}

static void gen_mips_call(Quad *qptr) {
    printf("    jal _%s\n", (qptr->src1->val).stptr->name);
    printf("    la $sp, %d($sp)\n", 4 * (qptr->src2->val).numval);
}

static void gen_mips_ret(Quad *qptr) {
    if (qptr->src1 != NULL) {
        gen_mips_load(qptr->src1, "v", 0);
    }

    printf("    la $sp, 0($fp)     # deallocate locals\n");
    printf("    lw $ra, 0($sp)     # restore return address\n");
    printf("    lw $fp, 4($sp)     # restore frame pointer\n");
    printf("    la $sp, 8($sp)     # restore stack pointer\n");
    printf("    jr $ra\n");
}

static void gen_mips_retrieve(Quad *qptr) {
    gen_mips_store(qptr->src1, "v", 0);
}

static void gen_mips_load(Operand *src, char *reg_prefix, int dstreg) {
    symtabnode *sptr;
    int type;

    switch (src->optype) {
        case INTEGER:
            printf("    li $%s%d, %d\n", reg_prefix, dstreg, src->val.numval);
            break;

        case SYMTBL_PTR:
            sptr = src->val.stptr;
            type = sptr->type;

            if (sptr->scope == Global) {
                printf("    %s $%s%d, _%s\n",
                       LoadIns[type], reg_prefix, dstreg, sptr->name);
            } else {
                assert(sptr->scope == Local);
                if (type == t_Array && sptr->formal) {
                    /*
                     * arrays are passed by reference, so the value passed is the address
                     */
                    type = t_Addr;
                }

                printf("    %s $%s%d, %d($fp)\t# %s\n",
                       LoadIns[type], reg_prefix, dstreg, sptr->offset, sptr->name);
            }
            break;

        case ADDR:
            sptr = src->val.stptr;
            if (sptr->scope == Global) {
                printf("    la $%s%d, _%s\n", reg_prefix, dstreg, sptr->name);
            } else {
                assert(sptr->scope == Local);
                if (sptr->formal) {
                    /*
                     * arrays are passed by reference, so the value passed is the address
                     */
                    printf("    lw $%s%d, %d($fp)\t# addr(%s)\n",
                           reg_prefix,
                           dstreg,
                           sptr->offset,
                           sptr->name);
                } else {
                    printf("    la $%s%d, %d($fp)\t# addr(%s)\n",
                           reg_prefix,
                           dstreg,
                           sptr->offset,
                           sptr->name);
                }
            }
            break;

        case DEREF:
            sptr = src->val.stptr;
            if (sptr->scope == Global) {
                printf("    lw $%s%d, _%s\n", reg_prefix, dstreg, sptr->name);
                printf("    %s $%s%d, 0(%s%d)\t# deref(%s)\n",
                       LoadIns[sptr->elt_type], reg_prefix, dstreg, reg_prefix, dstreg, sptr->name);
            } else {
                assert(sptr->scope == Local);
                printf("    lw $%s%d, %d($fp)\t# %s\n",
                       reg_prefix, dstreg, sptr->offset, sptr->name);
                printf("    %s $%s%d, 0($%s%d)\t# deref(%s)\n",
                       LoadIns[sptr->elt_type], reg_prefix, dstreg, reg_prefix, dstreg, sptr->name);
            }
            break;


        default:
            fprintf(stderr, "ERROR [%s]: Unrecognized operand type %d\n",
                    __func__, src->optype);
            exit(1);
    }
}

static void gen_mips_store(Operand *dst, char *reg_prefix, int srcreg) {
    symtabnode *sptr;

    switch (dst->optype) {
        case SYMTBL_PTR:
            sptr = dst->val.stptr;
            if (sptr->scope == Global) {
                printf("    %s $%s%d, _%s\n",
                       StoreIns[sptr->type], reg_prefix, srcreg, sptr->name);
            } else {
                assert(sptr->scope == Local);
                printf("    %s $%s%d, %d($fp)\t# %s\n",
                       StoreIns[sptr->type], reg_prefix, srcreg, sptr->offset, sptr->name);
            }
            break;

        default:
            fprintf(stderr, "ERROR [%s]: Unrecognized operand type %d\n",
                    __func__, dst->optype);
            exit(1);
    }
}


/*******************************************************************************
 *                                                                             *
 *                                  UTILITIES                                  *
 *                                                                             *
 *******************************************************************************/

char *op_names[] = {
        "no_op",
        "move",
        "goto",
        "if_eq",
        "if_ne",
        "if_le",
        "if_lt",
        "if_ge",
        "if_gt",
        "label",
        "enter",
        "leave",
        "param",
        "call",
        "ret",
        "retrieve",
        "add",
        "sub",
        "mul",
        "div",
        "uminus",
};

/*
 * mk_intcon_op(numval) -- create an operand with optype INTEGER and value numval.
 */
static Operand *mk_intcon_op(int numval) {
    Operand *otmp = zalloc(sizeof(Operand));
    otmp->optype = INTEGER;
    otmp->val.numval = numval;
    return otmp;
}

/*
 * mk_stptr_op(stptr) -- create an operand with optype SYMTBL_PTR and value stptr.
 */
static Operand *mk_stptr_op(symtabnode *stptr) {
    Operand *otmp = zalloc(sizeof(Operand));
    otmp->optype = SYMTBL_PTR;
    otmp->val.stptr = stptr;
    return otmp;
}

/*
 * mk_insptr_op(insptr) -- create an operand with optype INS_PTR and value insptr.
 */
static Operand *mk_insptr_op(Quad *insptr) {
    Operand *otmp = zalloc(sizeof(Operand));
    otmp->optype = INS_PTR;
    otmp->val.insptr = insptr;
    return otmp;
}

/*
 * mk_addr_op(insptr) -- create an operand with optype ADDR and value stptr.
 */
static Operand *mk_addr_op(symtabnode *stptr) {
    Operand *otmp = zalloc(sizeof(Operand));
    otmp->optype = ADDR;
    otmp->val.stptr = stptr;
    return otmp;
}


/*
 * mk_deref_op(insptr) -- create an operand with optype DEREF and value stptr.
 */
static Operand *mk_deref_op(symtabnode *stptr) {
    Operand *otmp = zalloc(sizeof(Operand));
    otmp->optype = DEREF;
    otmp->val.stptr = stptr;
    return otmp;
}


/*
 * mk_expr_op(sptr) -- If sptr has type t_Char or t_Int, creates and returns a
 * SYMTBL_PTR operand.  If sptr has type t_Addr, creates and returns a DEREF op.
 */
static Operand *mk_expr_op(symtabnode *stptr) {
    switch (stptr->type) {
        case t_Char:
        case t_Int:
            return mk_stptr_op(stptr);

        case t_Addr:
            return mk_deref_op(stptr);

        case t_Array:
            return mk_addr_op(stptr);

        default:
            fprintf(stderr, "ERROR [%s]: Unrecognized optype %d\n", __func__, stptr->type);
            exit(1);
    }
}


/*
 * new_instr(op, src1, src2, dst) -- create a new three-address instruction
 * (i.e., a Quad data structure), set its fields to the arguments given, and
 * return the result.
 */
static Quad *new_instr(Op op, Operand *src1, Operand *src2, Operand *dst) {
    Quad *qtmp = zalloc(sizeof(Quad));
    qtmp->op = op;
    qtmp->src1 = src1;
    qtmp->src2 = src2;
    qtmp->dst = dst;
    return qtmp;
}


/*
 * If ast is an array expression, gen_3addr_code will generate code to compute
 * its address.  deref_lvalue() adds a dereference instruction at the end.
 */
static void deref_lvalue(tnode *ast) {
    symtabnode *ast_val, *array;
    Quad *deref_ins;

    if (ast->ntype == ArraySubscript) {
        array = stArraySubscript_Array(ast);
        ast_val = new_temp(array->elt_type);
        deref_ins = new_instr(MOVE,
                              mk_deref_op(ast->place),
                              NULL,
                              mk_stptr_op(ast_val));

        /* add deref_ins to the end of ast's instruction list */
        ast->code_tl->next = deref_ins;
        ast->code_tl = deref_ins;
        ast->place = ast_val;
    }
}


#define MAX_TMP_NAMELEN  20
static int tmpname_ctr = 0;

static symtabnode *new_temp(int type) {
    char tmp_name_buf[MAX_TMP_NAMELEN];
    snprintf(tmp_name_buf, MAX_TMP_NAMELEN, "tmp$%d", tmpname_ctr++);
    symtabnode *stptr = SymTabInsert(strdup(tmp_name_buf), Local);
    stptr->type = type;
    return stptr;
}

static int label_ctr = 0;

static Quad *new_label() {
    return new_instr(LABEL, mk_intcon_op(label_ctr++), NULL, NULL);
}

/*******************************************************************************
 *                                                                             *
 *                                 PRINT ROUTINES                              *
 *                                                                             *
 *******************************************************************************/

static void print_3addr_op(Operand *operand) {
    if (operand == NULL) {
        return;
    }

    switch (operand->optype) {
        case INTEGER:
            printf("%d", operand->val.numval);
            break;

        case SYMTBL_PTR:
            printf("%s", (operand->val.stptr)->name);
            break;

        case INS_PTR:
            print_3addr_ins_no_nl(operand->val.insptr);
            break;

        case ADDR:
            printf("addr(%s)", (operand->val.stptr)->name);
            break;

        case DEREF:
            printf("deref(%s)", (operand->val.stptr)->name);
            break;

        default:
            fprintf(stderr, "*** [%s] Unrecognized operand type: %d\n",
                    __func__,
                    operand->optype);
    }

}

static void print_3addr_ins_no_nl(Quad *ins) {
    assert(ins != NULL);

    printf("%s ", op_names[ins->op]);
    print_3addr_op(ins->src1);

    if (ins->src1 != NULL && (ins->src2 != NULL || ins->dst != NULL)) {
        printf(", ");
    }

    print_3addr_op(ins->src2);

    if (ins->src2 != NULL && ins->dst != NULL) {
        printf(", ");
    }

    print_3addr_op(ins->dst);
}

void print_3addr_ins(Quad *ins) {
    print_3addr_ins_no_nl(ins);
    printf("\n");
}


void print_3addr_inslist(Quad *inslist) {
    while (inslist != NULL) {
        print_3addr_ins(inslist);
        inslist = inslist->next;
    }

}

/*
 * print_fixed_MIPS_code() prints out some necessary fixd MIPS code: the
 * instructions for the println function, and the initial jump to _main
 */
static void print_fixed_MIPS_code() {
    printf(".align 2\n");
    printf(".data\n");
    printf("__nl: .asciiz \"\\n\"\n");
    printf(".align 2\n");
    printf(".text\n");
    printf("# println: print out an integer followed by a newline\n");
    printf(" _println:\n");
    printf("    li $v0, 1\n");
    printf("    lw $a0, 0($sp)\n");
    printf("    syscall\n");
    printf("    li $v0, 4\n");
    printf("    la $a0, __nl\n");
    printf("    syscall\n");
    printf("    jr $ra\n");
    printf("\n");
    printf("# jump to _main, where user code starts, at start of execution\n");
    printf("main:\n");
    printf("    j _main\n");
    printf("\n");
}


/*
 * print_string_table() -- print out the string table
 */
static void print_string_table() {
    for (str_tmp = string_tbl; str_tmp != NULL; str_tmp = str_tmp->next) {
        printf("%s: .asciiz \"%s\"\n", str_tmp->str_lbl, str_tmp->str);
    }
}

/*******************************************************************************
 *                                                                             *
 *                             Code Optimization                               *
 *                                                                             *
 *******************************************************************************/

static void gen_optimize(tnode *ast) {
    Quad *code_hd = ast->code_hd;
    if (oLocalFlag) {
        peephole_optimize(code_hd);
        copy_propagate(code_hd);
    }
//    if (oglobal_flag){
//    }
}

static void peephole_optimize(Quad *fn_head) {
    for (Quad *curr = fn_head; curr != NULL; curr = curr->next) {
        switch (curr->op) {
            // control flow improve
            case IF_EQ:
            case IF_NE:
            case IF_LE:
            case IF_LT:
            case IF_GE:
            case IF_GT:
                if (curr->next == NULL || curr->next->next == NULL) {
                    break;
                }
                if (curr->next->op != GOTO || curr->next->next->op != LABEL) {
                    break;
                }
                if (curr->dst->val.insptr->src1->val.numval == curr->next->next->src1->val.numval) {
                    switch (curr->op) {
                        case IF_EQ:
                            curr->op = IF_NE;
                            break;
                        case IF_NE:
                            curr->op = IF_EQ;
                            break;
                        case IF_GT:
                            curr->op = IF_LE;
                            break;
                        case IF_LE:
                            curr->op = IF_GT;
                            break;
                        case IF_LT:
                            curr->op = IF_GE;
                            break;
                        case IF_GE:
                            curr->op = IF_LT;
                            break;
                        case MUL_OP:
                            curr->op = DIV_OP;
                            break;
                        case DIV_OP:
                            curr->op = MUL_OP;
                            break;
                        case ADD_OP:
                            curr->op = SUB_OP;
                            break;
                        case SUB_OP:
                            curr->op = ADD_OP;
                            break;
                        default:
                            curr->op = NOP;
                            break;
                    }
                    curr->dst = curr->next->dst;
                    curr->next = curr->next->next;
                }
                break;
            case ADD_OP:
                // i = 0 + ? => i = ?
                if (curr->src1->optype == INTEGER && curr->src1->val.numval == 0) {
                    curr->op = MOVE;
                    curr->src1 = curr->src2;
                    curr->src2 = NULL;
                }
                    // i = ? + 0 => i = ?
                else if (curr->src2->optype == INTEGER && curr->src2->val.numval == 0) {
                    curr->op = MOVE;
                    curr->src2 = NULL;
                }
                break;
            case SUB_OP:
                // i = 0 - ? => i = -?
                if (curr->src1->optype == INTEGER && curr->src1->val.numval == 0) {
                    curr->op = UMINUS_OP;
                    curr->src1 = curr->src2;
                    curr->src2 = NULL;
                }
                    // i = ? - 0 => i = ?
                else if (curr->src2->optype == INTEGER && curr->src2->val.numval == 0) {
                    curr->op = MOVE;
                    curr->src2 = NULL;
                }
                break;
            case MUL_OP:
                // i = 1 * ? => i = ?
                if (curr->src1->optype == INTEGER && curr->src1->val.numval == 1) {
                    curr->op = MOVE;
                    curr->src1 = curr->src2;
                    curr->src2 = NULL;
                }
                    // i = ? * 1 => i = ?
                else if (curr->src2->optype == INTEGER && curr->src2->val.numval == 1) {
                    curr->op = MOVE;
                    curr->src2 = NULL;
                }
                    // i = ? * 0 => i = 0
                else if (curr->src1->optype == INTEGER && curr->src1->val.numval == 0) {
                    curr->op = MOVE;
                    curr->src2 = NULL;
                }
                    // i = 0 * ? => i = 0
                else if (curr->src2->optype == INTEGER && curr->src2->val.numval == 0) {
                    curr->op = MOVE;
                    curr->src1 = curr->src2;
                    curr->src2 = NULL;
                }
                break;
            case DIV_OP:
                // i = 0 / ? => i = 0
                if (curr->src1->optype == INTEGER && curr->src1->val.numval == 0) {
                    curr->op = MOVE;
                    curr->src2 = NULL;
                }
                    // i = ? / 1 => i = ?
                else if (curr->src2->optype == INTEGER && curr->src2->val.numval == 1) {
                    curr->op = MOVE;
                    curr->src2 = NULL;
                }
                break;
            default:
                break;
        }
    }
}


static void copy_propagate(Quad *fn_head) {
    for (Quad *curr = fn_head; curr != NULL; curr = curr->next) {
        if (curr->op != MOVE) {
            continue;
        }
        Operand *lhs = curr->dst;
        Operand *rhs = curr->src1;

        // lhs char; rhs int; do NOT copy propagate
        // due to char size is smalled than int size, this will lose upper data
        if (lhs->val.stptr->type == t_Char && rhs->val.stptr->type == t_Int) {
            continue;
        }

        for (Quad *propagating_curr = curr->next; propagating_curr != NULL; propagating_curr = propagating_curr->next) {
            // reach the end of block
            if (propagating_curr->op == LABEL) {
                break;
            }
            // lhs is global and enter another functions, do NOT copy propagate
            // due to another functions may change the global value
            if (lhs->val.stptr->scope == Global && propagating_curr->op == CALL) {
                break;
            }
            // replace
            if (propagating_curr->op == MUL_OP || propagating_curr->op == DIV_OP ||
                propagating_curr->op == ADD_OP || propagating_curr->op == SUB_OP ||
                propagating_curr->op == MOVE || propagating_curr->op == UMINUS_OP) {
                // lhs is redefined OR rhs is redefined, do NOT copy propagate
                if (operands_compare(lhs, propagating_curr->dst) ||
                    operands_compare(rhs, propagating_curr->dst)) {
                    break;
                }
                // type different, continue to look for next
                if (lhs->val.stptr->type != propagating_curr->dst->val.stptr->type) {
                    continue;
                }
                if (operands_compare(lhs, propagating_curr->src1)) {
                    propagating_curr->src1 = rhs;
                }
                if (operands_compare(lhs, propagating_curr->src2)) {
                    propagating_curr->src2 = rhs;
                }
            }
        }
    }
}

static int operands_compare(Operand *a, Operand *b) {
    if (a == NULL || b == NULL) {
        return 0;
    }
    if (a->optype == SYMTBL_PTR && b->optype == SYMTBL_PTR) {
        return !strcmp(a->val.stptr->name, b->val.stptr->name);
    }
    return 0;
}