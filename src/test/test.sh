#!/bin/bash

i=0
while (( $? == 0 ))
do
  ((i++))
  echo "Case ${i}"
  rm bpt_file*
  rm file.txt
  rm other.txt
  python3 makedata_bpt.py
  echo "running std."
  ./std_test <test1.txt >std.out
  echo "running test"
  ./code <test1.txt >test.out
  echo "comparing"
  diff test.out std.out >/dev/null

  python3 makedata2_bpt.py
  echo "running std."
  ./std_test <test2.txt >std.out
  echo "running test"
  ./code <test2.txt >test.out
  echo "comparing"
  diff test.out std.out >/dev/null
done