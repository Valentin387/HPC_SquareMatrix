#! /bin/bash
rm timesLeidy.doc
for j in {1..10}
do
for i in 10000 500000 40000000 300000000000 99999999999 
do
./ExeLeidy $i >> timesLeidy.doc
done
done
