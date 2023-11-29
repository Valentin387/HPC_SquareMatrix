#! /bin/bash
rm timesMatricesValentin.doc
for j in {1..10}
do
for i in 102 501 801 900 1002 2001
do
mpirun -n 3 -hostfile hostfile.txt ./exefile $i 0 >> timesMatricesValentin.doc
done
done
