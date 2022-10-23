#include "global.h"
#include "symbol-table.h"

extern int yydebug;
extern int yyparse();

int status = 0;
int oLocalFlag=0;
int oGlobalFlag=0;

int main(int argc, char *argv[])
{
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-Olocal") == 0)
            oLocalFlag = 1;
        else if (strcmp(argv[i], "-Oglobal") == 0)
            oGlobalFlag = 1;
    }
  SymTabInit(Global);
  SymTabInit(Local);

  if (yyparse() < 0) {
    printf("main: syntax error\n");
    status = 1;
  }

  return status;
}
