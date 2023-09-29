#! /bin/bash
rm times2.doc
for j in {1..10}
do
for i in 100 
do
./ExeValentin $i >> timesValentin.doc
done
done
