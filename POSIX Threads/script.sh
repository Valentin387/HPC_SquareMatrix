#! /bin/bash
rm times16Threads.doc
for j in {1..10}
do
for i in 100 500 800 900 1000 2000
do
./ExeValentin $i 0 >> times16Threads.doc
done
done
