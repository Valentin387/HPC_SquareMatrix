#! /bin/bash
rm times2ProcessLeidy.doc
for j in {1..10}
do
for i in 100 500 800 900 1000 2000
do
./ExeLeidy $i 0 >> times2ProcessLeidy.doc
done
done
