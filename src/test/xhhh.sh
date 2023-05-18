#!/bin/bash

rm -rf data
rm -rf files
./code.conless <./testcases/basic_3/1.in >test.out.conless
./code.conless <./testcases/basic_3/2.in >test.out.conless
./code.conless <./testcases/basic_3/3.in >test.out.conless
./code.conless <./test.3.in >test.out.conless
./code <./testcases/basic_3/1.in >test.out
./code <./testcases/basic_3/2.in >test.out
./code <./testcases/basic_3/3.in >test.out
./code <./test.3.in >test.out
