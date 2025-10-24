# minishell

## How to Run

1. flex lex.c
2. gcc myshell.c lex.yy.c -lfl -o myshell
3. ./myshell

Or use make

## Test result

SUCCESS:
    tr 'a-mn-z' 'n-za-m' < testfile | sort
    tr 'a-mn-z' 'n-za-m' < testfile | sort | wc -l
    who && ls
    ls file1 && who (make sure ls fails by not having a file named file1 in your directory)
    who || ls file1
    ls file1 || who
    sort < testfile > outfile
    cat testfile | sed 's/schedul/xxxxx/g'
    cat testfile | sed 's/schedul/xxxxx/g' > outfile

FAIL:
    top &
    sleep 3; ls &
