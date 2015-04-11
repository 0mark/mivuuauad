#!/bin/bash

function mp_rotary_daemon {
    while [ 1 ]; do
        read -r line < /dev/ttyS0
        #echo "Read: -$line-"
        #line=$(echo $line | tr -d '\n')
        line=$(echo $line | sed 's/[^a-z]//')
        case "$line" in
            click)
                #echo "click"
                mpc -q toggle
                ;;
            left)
                #echo "left"
                mpc -q volume -5
                ;;
            right)
                #echo "right"
                mpc -q volume +5
                ;;
        esac
        line=''
    done
}

mp_rotary_daemon </dev/null >/dev/null 2>&1 &
disown

#mp_rotary_daemon
