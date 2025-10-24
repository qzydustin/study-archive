#!/usr/bin/env csh

set PROGS = "globalopt-01 globalopt-02 globalopt-03 globalopt-04 globalopt-05 globalopt-06 globalopt-07 globalopt-08 globalopt-09 globalopt-10"


set SPIM = $HOME/courses/spr21/PROJECT/spim-files/spim.exe

foreach f ( $PROGS )
    echo $f
    ../REF-IMPL/compile < $f > zzz.s
    $SPIM -f zzz.s > $f-out
    echo "exit status: $status" >> $f-out
    /bin/rm zzz.s
end


