#!/usr/bin/env csh

set PROGS = "test01 test02 test03 test04 test05 test06 test07 test08 test09 test10 test11 test12 test13 test14 test15 test16 test17 test18 test19 test20 test21 test22 test23 test24 test25 test26 test27 test28 test29 test30 test31 test32 test33"

set SPIM = $HOME/courses/spr21/PROJECT/spim-files/spim.exe

foreach f ( $PROGS )
    echo $f
    ../REF-IMPL/compile < $f > zzz.s
    $SPIM -f zzz.s > $f-out
    /bin/rm zzz.s
end


