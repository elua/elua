#!/bin/bash

# Build platform docs
for f in arch_platform/arch_platform_*.lua
do
  fname=$(basename $f)
  if [ $fname == "arch_platform_template.lua" ]
  then
    echo "Skipping $fname"
    echo
    continue
  fi
  newfname=$(echo $fname | sed "s/\.lua/\.html/")
  lua builddoc.lua $f
  if [ $? -eq 0 ]
  then
    mv arch_platform/$newfname en/
    echo "DONE processing $f"
  else
   echo "**** Building $f failed"
  fi
  echo
done

