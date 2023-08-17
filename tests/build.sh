#!/bin/sh

set -u

MEM='test-memory'
STR='test-strings'
PLT='test-platform'
DYN='add'
CompileOpts='-I../src -g -Wall -mavx2 -fpermissive -lm -w'

mkdir -p ../build
cd ../build
gcc -o ${MEM} ../tests/${MEM}.c ${CompileOpts}
gcc -o ${STR} ../tests/${STR}.c ${CompileOpts}
g++ -o ${PLT} ../tests/${PLT}.cpp ${CompileOpts}
gcc -o ${DYN}.so ../tests/${DYN}.c ${CompileOpts} -shared
cd ../tests