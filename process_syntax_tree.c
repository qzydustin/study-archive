/*
 * File: process_syntax_tree.c
 * Purpose: Code to traverse the syntax tree of each function and
 *     perform any desired actions.
 */

#include <stdio.h>
#include "syntax-tree.h"
#ifdef COUNT
#include "process_node_count.c"
#endif
#include "process_mips_code.c"

extern void printSyntaxTree(tnode *t, int n, int depth);

instr *instrHead = NULL;
instr *instrTail = NULL;
instr *globalTail = NULL;


/*
 * process_syntax_tree(fn_name, fn_body) -- fn_name is a poiner to
 * the symbol table entry of the function being processed (which can
 * be used, for example, to get the name of the fucntion); fn_body
 * is a pointer to the root of the syntax tree of the function body.
 * This function can be used to carry out tree traversals of the
 * function's syntax tree.
 */
void process_syntax_tree(symtabnode *fn_name, tnode *fn_body)
{
#ifdef DEBUG
  printf("@@FUN: %s\n", fn_name->name);
  printf("@@BODY:\n");
  printSyntaxTree(fn_body, 4, 0);
  printf("-----\n");
#endif

#ifdef COUNT
  // syntax tree node count
  printf("@@FUN: %s\n", fn_name->name);
  int nodeCount[SyntaxNodeTypeSize] = {0};
  calculateSyntaxTreeNodeCount(fn_body, nodeCount);
  printSyntaxTreeNodeCount(nodeCount);
#endif
  instrHead = NULL;
  instrTail = NULL;
  globalTail = NULL;
  newInstr(OpEnter, NULL, fn_name, NULL);
  generateThreeAddressCode(fn_body);
  int no = 0;
  generateMipsCode(fn_name, instrHead);
}
