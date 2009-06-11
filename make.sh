#!/bin/sh

make -C simulator -f Makefile.opt
make -C solution/ants_engine/
make -C Bertl/SDL
make -C optimizer
