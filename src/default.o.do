#!/bin/sh

deps=$2.deps
deps_ne=$2.deps_ne
cflags="-O2 -MD -MF $deps"

if (command -v strace >/dev/null); then
  strace -e stat,stat64,fstat,fstat64,lstat,lstat64 -f 2>&1 >/dev/null \
   gcc $cflags -o $3 -c ${1%.o}.c \
   |grep '1 ENOENT'\
   |grep '\.h'\
   |cut -d'"' -f2\
   >$deps_ne

  while read -r DEP_NE; do
    redo-ifcreate ${DEP_NE}
  done <$deps_ne
else
  (
    IFS=:
    for folder in $PATH; do
      redo-ifcreate $folder/strace
    done
  )
  g++ -std=c++14 $cflags -o $3 -c ${1%.o}.cpp \
  -I. \
  -I./deps/imgui/ \
  -I./deps/json/ 

read DEPS <$deps
redo-ifchange ${DEPS#*:}

fi
