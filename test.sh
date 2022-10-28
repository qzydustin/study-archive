time (cat testcase | ./compile > testcase.out )&& spim-stats/spim/spim -keepstats -f testcase.out
time (cat testcase | ./compile -Olocal > testcase.out )&& spim-stats/spim/spim -keepstats -f testcase.out
time (cat testcase | ./compile -Oglobal > testcase.out )&& spim-stats/spim/spim -keepstats -f testcase.out
time (cat testcase | ./compile -Olocal -Oglobal > testcase.out )&& spim-stats/spim/spim -keepstats -f testcase.out
