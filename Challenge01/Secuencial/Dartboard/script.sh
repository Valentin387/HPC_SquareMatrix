#! /bin/bash
rm timesValentinO2.doc
for j in {1..10}
do
for i in 1000 20000 3000000 50000000 80000000
do
./ExeValentinO2 $i >> timesValentin02.doc
done
done
