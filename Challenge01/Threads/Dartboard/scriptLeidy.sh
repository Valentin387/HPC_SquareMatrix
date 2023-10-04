#! /bin/bash
rm times16Threads.doc
for j in {1..10}
do
for i in 1000 20000 3000000 50000000 80000000
do
./ExeLeidy $i 0 >> times16Threads.doc
done
done
