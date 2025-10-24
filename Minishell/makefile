main: myshell.o lex.yy.o
	gcc -o myshell myshell.o lex.yy.o -lfl
lex.yy.c: lex.c
	flex lex.c
myshell.o: myshell.c
	gcc -c myshell.c
lex.yy.o: lex.yy.c
	gcc -c lex.yy.c