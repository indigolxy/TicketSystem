#!/bin/bash

i=0
while (( $? == 0 ))
do
  ((i++))
  echo "Case ${i}"
  rm file
  rm file.txt
  rm other.txt
  python3 makedata_bpt.py
  echo "running std."
  ./std_test <test.txt >std.out
  echo "running test"
  ./code <test.txt >test.out
  echo "comparing"
  diff test.out std.out >/dev/null
done