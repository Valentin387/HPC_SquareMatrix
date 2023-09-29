#! /bin/bash
rm times2.doc
for j in {1..10}
do
for i in 1000 20000 3000000 500000000 800000000
do
./ExeValentin $i 0 >> timesValentin.doc
done
done
