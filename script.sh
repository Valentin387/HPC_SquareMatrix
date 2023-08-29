#! /bin/bash
rm times2.doc
for j in {1..10}
do
for i in 10 100 200 400 600 800 1000 2000
do
./ubuntuEXE $i 0 >> times2.doc
done
done
