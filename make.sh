#!/bin/sh

make -C simulator -f Makefile.opt
make -C Bertl/SDL
make -C optimizer
