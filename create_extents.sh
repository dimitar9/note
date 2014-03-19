#!/bin/bash
if [ $# -ne 2 ]
then
    echo "$0 [filename] [size in kb]"
fi
filename=$1
size=$2
i=0
while [ $i -lt $size ]
do
    i=`expr $i +  7`
    echo -n "$i" | dd of=$1 bs=1024 seek=$i
done


