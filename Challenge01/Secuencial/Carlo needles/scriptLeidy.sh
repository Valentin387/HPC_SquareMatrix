#! /bin/bash
rm timesLeidy.doc
for j in {1..5}
do
for i in 10000
do
./ExeLeidy 2 1 $i >> timesLeidy.doc
done
done
