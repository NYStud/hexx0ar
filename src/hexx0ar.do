#!/bin/sh

FILES=$(find . -type f -name '*.cpp')
for file in $FILES; do
  redo-ifchange ${file%.*}.o
done
OBJECTS=$(find . -type f -name '*.o')
g++ -std=c++14 -O2 -o $3 $OBJECTS -lm -lSDL2 -lSDL2_image -lepoxy -llua5.3 -lboost_filesystem -lboost_system -lboost_program_options
