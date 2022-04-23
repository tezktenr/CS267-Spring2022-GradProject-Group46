#!/bin/sh

correctFile=${1}
testFile=${2}

sort ${correctFile} -o correct_sort.csv
sort ${testFile} -o test_sort.csv

diff correct_sort.csv test_sort.csv

rm correct_sort.csv
rm test_sort.csv