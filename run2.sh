#!/bin/sh
# world red black 

export WTF=/tmp/haeusl-runner.wtf

trap "rm -rf $WTF"

./simulator/simulator $1 $2 $3 wtf > $WTF && ./Bertl/SDL/ants_sdl $1 >&/dev/null $WTF

rm -f $WTF
