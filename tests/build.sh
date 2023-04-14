#!/bin/sh

set -u

MEM='test-memory'
STR='test-strings'
PLT='test-platform'
CompileOpts='-I../src -g -Wall -mavx2 -fpermissive -w'

mkdir -p ../build
cd ../build
g++ -o ${MEM} ../tests/${MEM}.c ${CompileOpts}
g++ -o ${STR} ../tests/${STR}.c ${CompileOpts}
g++ -o ${PLT} ../tests/${PLT}.cpp ${CompileOpts}
cd ../tests