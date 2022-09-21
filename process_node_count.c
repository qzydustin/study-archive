#include <stdarg.h>
#include "global.h"
#include "protos.h"
#include "syntax-tree.h"

#define SyntaxNodeTypeSize 27

void calculateSyntaxTreeNodeCount(tnode *t, int *nodeCount)
{
  tnode *tntmp0;

  if (t == NULL)
  {
    return;
  }

  switch (t->ntype)
  {
  case Error:
    nodeCount[Error]++;
    break;

  case Intcon:
    nodeCount[Intcon]++;
    break;

  case Charcon:
    nodeCount[Charcon]++;
    break;

  case Stringcon:
    nodeCount[Stringcon]++;
    break;

  case Var:
    nodeCount[Var]++;
    break;

  case ArraySubscript:
    nodeCount[ArraySubscript]++;
    calculateSyntaxTreeNodeCount(stArraySubscript_Subscript(t), nodeCount);
    break;

  case UnaryMinus:
    nodeCount[UnaryMinus]++;
    calculateSyntaxTreeNodeCount(stUnop_Op(t), nodeCount);
    break;

  case LogicalNot:
    nodeCount[LogicalNot]++;
    calculateSyntaxTreeNodeCount(stUnop_Op(t), nodeCount);
    break;

  case Plus:
    nodeCount[Plus]++;
    calculateSyntaxTreeNodeCount(stBinop_Op1(t), nodeCount);
    calculateSyntaxTreeNodeCount(stBinop_Op2(t), nodeCount);
    break;
  case BinaryMinus:
    nodeCount[BinaryMinus]++;
    calculateSyntaxTreeNodeCount(stBinop_Op1(t), nodeCount);
    calculateSyntaxTreeNodeCount(stBinop_Op2(t), nodeCount);
    break;
  case Mult:
    nodeCount[Mult]++;
    calculateSyntaxTreeNodeCount(stBinop_Op1(t), nodeCount);
    calculateSyntaxTreeNodeCount(stBinop_Op2(t), nodeCount);
    break;
  case Div:
    nodeCount[Div]++;
    calculateSyntaxTreeNodeCount(stBinop_Op1(t), nodeCount);
    calculateSyntaxTreeNodeCount(stBinop_Op2(t), nodeCount);
    break;
  case Equals:
    nodeCount[Equals]++;
    calculateSyntaxTreeNodeCount(stBinop_Op1(t), nodeCount);
    calculateSyntaxTreeNodeCount(stBinop_Op2(t), nodeCount);
    break;
  case Neq:
    nodeCount[Neq]++;
    calculateSyntaxTreeNodeCount(stBinop_Op1(t), nodeCount);
    calculateSyntaxTreeNodeCount(stBinop_Op2(t), nodeCount);
    break;
  case Leq:
    nodeCount[Leq]++;
    calculateSyntaxTreeNodeCount(stBinop_Op1(t), nodeCount);
    calculateSyntaxTreeNodeCount(stBinop_Op2(t), nodeCount);
    break;
  case Lt:
    nodeCount[Lt]++;
    calculateSyntaxTreeNodeCount(stBinop_Op1(t), nodeCount);
    calculateSyntaxTreeNodeCount(stBinop_Op2(t), nodeCount);
    break;
  case Geq:
    nodeCount[Geq]++;
    calculateSyntaxTreeNodeCount(stBinop_Op1(t), nodeCount);
    calculateSyntaxTreeNodeCount(stBinop_Op2(t), nodeCount);
    break;
  case Gt:
    nodeCount[Gt]++;
    calculateSyntaxTreeNodeCount(stBinop_Op1(t), nodeCount);
    calculateSyntaxTreeNodeCount(stBinop_Op2(t), nodeCount);
    break;
  case LogicalAnd:
    nodeCount[LogicalAnd]++;
    calculateSyntaxTreeNodeCount(stBinop_Op1(t), nodeCount);
    calculateSyntaxTreeNodeCount(stBinop_Op2(t), nodeCount);
    break;
  case LogicalOr:
    nodeCount[LogicalOr]++;
    calculateSyntaxTreeNodeCount(stBinop_Op1(t), nodeCount);
    calculateSyntaxTreeNodeCount(stBinop_Op2(t), nodeCount);
    break;
  case FunCall:
    nodeCount[FunCall]++;
    calculateSyntaxTreeNodeCount(stFunCall_Args(t), nodeCount);
    break;

  case Assg:
    nodeCount[Assg]++;
    calculateSyntaxTreeNodeCount(stAssg_Lhs(t), nodeCount);
    calculateSyntaxTreeNodeCount(stAssg_Rhs(t), nodeCount);
    break;

  case Return:
    nodeCount[Return]++;
    calculateSyntaxTreeNodeCount(stReturn(t), nodeCount);
    break;

  case For:
    nodeCount[For]++;
    calculateSyntaxTreeNodeCount(stFor_Init(t), nodeCount);
    calculateSyntaxTreeNodeCount(stFor_Test(t), nodeCount);
    calculateSyntaxTreeNodeCount(stFor_Update(t), nodeCount);
    calculateSyntaxTreeNodeCount(stFor_Body(t), nodeCount);
    break;

  case While:
    nodeCount[While]++;
    calculateSyntaxTreeNodeCount(stWhile_Test(t), nodeCount);
    calculateSyntaxTreeNodeCount(stWhile_Body(t), nodeCount);
    break;

  case If:
    nodeCount[If]++;
    calculateSyntaxTreeNodeCount(stIf_Test(t), nodeCount);
    calculateSyntaxTreeNodeCount(stIf_Then(t), nodeCount);
    calculateSyntaxTreeNodeCount(stIf_Else(t), nodeCount);
    break;

  case STnodeList: /* list of syntax tree nodes */
    nodeCount[STnodeList]++;
    /* iterate down the list, printing out each tree in the list in turn */
    for (tntmp0 = t; tntmp0 != NULL; tntmp0 = stList_Rest(tntmp0))
    {
      calculateSyntaxTreeNodeCount(stList_Head(tntmp0), nodeCount);
    }
    break;

  default:
    fprintf(stderr, "Unknown syntax tree node type %d\n", t->ntype);
  }

  return;
}

void printSyntaxTreeNodeCount(int *nodeCount)
{
  char *nodeTypeName[] =
      {
          "Error",
          "Intcon",
          "Charcon",
          "Stringcon",
          "Var",
          "ArraySubscript",
          "Plus",
          "UnaryMinus",
          "BinaryMinus",
          "Mult",
          "Div",
          "Equals",
          "Neq",
          "Leq",
          "Lt",
          "Geq",
          "Gt",
          "LogicalAnd",
          "LogicalOr",
          "LogicalNot",
          "FunCall",
          "Assg",
          "Return",
          "For",
          "While",
          "If",
          "STnodeList"};

  for (int i = 0; i < SyntaxNodeTypeSize; i++)
  {
    if (nodeCount[i] != 0)
    {
      printf("%s: %d \n", nodeTypeName[i], nodeCount[i]);
    }
  }
}
