#!/bin/bash
for i in {1..7};
do echo $i;
   ./check-pt-$i.sh
   # sync; echo 3 > /proc/sys/vm/drop_caches;
done;
