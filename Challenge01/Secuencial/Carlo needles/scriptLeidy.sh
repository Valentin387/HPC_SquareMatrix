#! /bin/bash
rm timesLeidyO3.doc
for j in {1..10}
do
for i in 1000 20000 3000000 50000000 80000000
do
./ExeLeidyO3 $i >> timesLeidyO3.doc
done
done
