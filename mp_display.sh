#!/bin/bash


function mp_display_daemon {
    ot=''
    oa=''
    ov=''
    sv=0

    while [ 1 ]; do
        t=$(mpc -f "%title%" | head -n 1 | cut -c 1-15)
        a=$(mpc -f "%artist%" | head -n 1 | cut -c 1-15)
        v=$(mpc | tail -n 1 | sed "s/volume:\s\+\([0-9]\+\).*/\1/g")
        if [ "$ot" != "$t" -o "$oa" != "$a" -o "$ov" != "$v" -o $sv -eq 1 ]; then
            #echo "$v - $ov"
            if [ "$ov" != "$v" ]; then
                #echo "p$a|Volume: $v%\n"
                echo -e "p$a|Volume: $v%\n" > /dev/ttyS0
                sv=1
            else
                #echo "p$a|$t\n"
                echo -e "p$a|$t\n" > /dev/ttyS0
                sv=0
            fi
            ot=$t
            oa=$a
            ov=$v
        fi
        sleep 1
    done
}

mp_display_daemon </dev/null >/dev/null 2>&1 &
disown
