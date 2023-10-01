#! /bin/bash
rm times16Valentin.doc
for j in {1..10}
do
for i in 1000 20000 3000000 50000000 80000000
do
./ExeValentin $i >> times16Valentin.doc
done
done
