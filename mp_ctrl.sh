#!/bin/bash

case "$1" in
    on)
        mpc -q volume 5
        echo -e "s10\n" > /dev/ttyS0
        sleep 15
        echo -e "s11\n" > /dev/ttyS0
        ;;
    off)
        mpc -q volume 5
        echo -e "s00\n" > /dev/ttyS0
        sleep 1
        echo -e "s01\n" > /dev/ttyS0
        ;;
    init)
        echo "+X+\n" > /dev/ttyS0
        ;;
esac