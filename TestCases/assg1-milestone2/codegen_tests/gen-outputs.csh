#!/usr/bin/env csh

set PROGS = "test01 test02 test03 test04 test05 test06 test07 test08 test09 test10 test11 test12 test13 test14 test15 test16 test17 test18 test19 test20 test21 test22 test23 test24 test25 test26 test27 test28 test29 test30 test31 test32 test33 test34 test35 test36 test37 test38 test39 test40 test41 test42 test43 test44 test45 test46 test47 test48 test49 test50"

set SPIM = $HOME/courses/spr21/PROJECT/spim-files/spim.exe

foreach f ( $PROGS )
    echo $f
    ../../REF-IMPL/compile < $f > zzz.s
    $SPIM -f zzz.s > $f-out
    echo "exit status: $status" >> $f-out
    /bin/rm zzz.s
end


