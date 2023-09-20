#! /bin/bash
rm timesLab2.doc
for j in {1..10}
do
for i in 2000
do
./ExeLab2 $i 0 >> timesLab2.doc
done
done
