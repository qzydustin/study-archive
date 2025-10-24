make clean
make realclean
make

cat testcase | ./compile> testcase.out && time(spim-stats/spim/spim -keepstats -f testcase.out)
cat testcase | ./compile -Oregalloc > testcase.out && time(spim-stats/spim/spim -keepstats -f testcase.out)
cat testcase | ./compile -Olocal > testcase.out && time(spim-stats/spim/spim -keepstats -f testcase.out)
cat testcase | ./compile -Oglobal > testcase.out && time(spim-stats/spim/spim -keepstats -f testcase.out)
cat testcase | ./compile -Olocal -Oregalloc > testcase.out > testcase.out && time(spim-stats/spim/spim -keepstats -f testcase.out)
cat testcase | ./compile -Oglobal -Oregalloc > testcase.out > testcase.out && time(spim-stats/spim/spim -keepstats -f testcase.out)
cat testcase | ./compile -Oglobal -Olocal > testcase.out > testcase.out && time(spim-stats/spim/spim -keepstats -f testcase.out)
cat testcase | ./compile -Oglobal  -Olocal -Oregalloc > testcase.out > testcase.out && time(spim-stats/spim/spim -keepstats -f testcase.out)
