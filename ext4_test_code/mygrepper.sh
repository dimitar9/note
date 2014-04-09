#!bin/bash

if [ $# -lt 1 ]
then
    echo "Usage: $0 <output> <input>" 
    exit 1
fi






touch $1
file=$2
while IFS= read -r line
do
    echo -n "$line\n" >> $1    
    grep -rnIi "$line" /home/paul/Documents/linuxwork/ext4/fs/ext4 >> $1 2>&1
    echo '\n' >> $1 2>&1
done <"$file"

sed -i '/^grep/d' $1  

