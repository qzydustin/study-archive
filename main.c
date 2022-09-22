#include "global.h"
#include "symbol-table.h"

extern int yydebug;
extern int yyparse();

int status = 0;
int tmpVarCount = 0;
int labelCount = 0;
int hasInit = 0;

int main(void)
{
  SymTabInit(Global);
  SymTabInit(Local);
  if (yyparse() < 0) {
    printf("main: syntax error\n");
    status = 1;
  }
  
  return status;
}
