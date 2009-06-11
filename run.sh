#!/bin/sh
# world red black 

export WTF=/tmp/haeusl-runner.wtf

trap "rm -rf $WTF"

./simulator/simulator $1 $2 $3 wtf > $WTF && ./solution/ants_engine/ants_sdl $1 $WTF

rm -f $WTF
