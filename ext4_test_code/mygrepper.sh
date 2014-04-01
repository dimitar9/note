#!bin/bash

if [ $# -lt 1 ]
then
    echo "Usage: $0 <output>" 
    exit 1
fi






touch $1
file="funcs_in_extents_status.data"
while IFS= read -r line
do
    echo -n "$line\n" >> $1    
    grep -rnIi "$line" /home/paul/Documents/linuxwork/ext4/fs/ext4 >> $1 2>&1
    echo '\n' >> $1 2>&1
done <"$file"

sed -i '/^grep/d' $1  

