#!/usr/bin/sh
who |tr -s ' ' | cut -d ' ' -f1 > login.txt
for line in $(cat login.txt); 
do 
ps -uax |sed -n '/^'$line'/p'|tr -s ' ' | cut -d ' ' -f5 > ram.txt;
python tets.py $line
done  