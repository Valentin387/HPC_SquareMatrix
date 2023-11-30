#! /bin/bash
rm timesNeedlesValentin.doc
for j in {1..10}
do
for i in 1000 20000 3000000 50000000 80000000
do
mpirun -n 3 -hostfile hostfile.txt ./exefile $i >> timesNeedlesValentin.doc
done
done
