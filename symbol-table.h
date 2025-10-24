/*
 * symbol-table.h
 *
 * Author: Saumya Debray
 */

#ifndef _SYMBOL_TABLE_H_
#define _SYMBOL_TABLE_H_

#define Global 0
#define Local  1
#define HASHTBLSZ 256


typedef struct stblnode {
  char *name;
  int scope;
  bool formal;             /* true if formal, false o/w */
  int type;                /* the type of the symbol */
  int elt_type;            /* the type of array elements */
  int num_elts;            /* no. of array elements */
  int ret_type;            /* the return type of a function */
  struct stblnode *formals;  /* the list of formals for a function */
  int nargs;               /* the no. of arguments of a function */
  int argpos;              /* if a variable is the n^th formal parameter, 
                              this has value n (n == 1 for the first formal); 
                              otherwise this value is 0. */
  int offset;              /* if the symbol is a local variable, 
                              this is its stack frame offset;
                              if it is a global function, it is the function's
                              stack frame size. */
    int fn_proto_state;      /* status of prototype definitions for a function */
    bool is_extern;          /* whether or not an ID was declared as an extern */
    bool skip_print;         /* for globals: skip printing the global if set to 1.
                              globals corresponding to string constant labels
                              have this value set to 1 since the string constant
                              is printed out of the string table. */
    int indexBit;             // the index of the variable in the bit vector
    double cost;                 //the cost of using a reg
    int regName;              //the num of the register ($s_)
    bool isReturn;
    struct stblnode *next;
} symtabnode;

/*********************************************************************
 *                                                                   *
 *                             Prototypes                            *
 *                                                                   *
 *********************************************************************/

void SymTabInit(int sc); // initialize the symbol table at scope sc to empty
symtabnode *SymTabLookup(char *str, int sc); // lookup scope sc
symtabnode *SymTabLookupAll(char *str); // lookup local first, then global
symtabnode *SymTabInsert(char *str, int sc);  // add ident to symbol table
symtabnode *SymTabRecordFunInfo(bool isProto);
void CleanupFnInfo(void);
/*
 * Code generation functions
 */
int allocate_storage(symtabnode *sptr);

void print_globals();

void dump_fn_info(symtabnode *sptr);

/*
 * Defugging functions
 */
void printSTNode(symtabnode *stptr);

void DumpSymTabLocal();

void DumpSymTabGlobal();

void DumpSymTab();

symtabnode *searchByIndex(int indexBit);

/*********************************************************************
 *                                                                   *
 *          Functions: states for prototypes and definitions         *
 *                                                                   *
 *********************************************************************/

#define FN_PROTO     0  // prototype seen
#define FN_DEFINED   1  // definition seen

#endif /* _SYMBOL_TABLE_H_ */
