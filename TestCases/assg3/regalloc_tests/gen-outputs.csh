#!/usr/bin/env csh

set PROGS = "`/bin/ls regalloc*[0-9]`"

set SPIM = $HOME/courses/spr21/PROJECT/spim-files/spim.exe

foreach f ( $PROGS )
    echo $f
    ../REF-IMPL/compile < $f > zzz.s
    $SPIM -f zzz.s > $f-out
    echo "exit status: $status" >> $f-out
    /bin/rm zzz.s
end


