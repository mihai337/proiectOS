#!/bin/bash

file_path=$1
safe_path=$2
filename=$(basename -- "$file_path")
safename=$(basename -- "$safe_path")

# if the file contains fewer than 3 lines and the number of words exceeds 1000 and the number of characters exceeds 2000, then the file is suspicious
# if the file is suspcious, check for non-ASCII characters and keywords like "attack" , "malware" , "virus" , "worm" , "trojan" , "backdoor" , "exploit" , "rootkit" , "spyware" , "adware" , "ransomware" , "keylogger" , "phishing" , "social engineering" , "denial of service" , "distributed denial

if [ $(sudo wc -l $file_path | cut -d ' ' -f 1) -lt 3 ] && [ $(sudo wc -w $file_path | cut -d ' ' -f 1) -gt 1000 ] && [ $(sudo wc -m $file_path | cut -d ' ' -f 1) -gt 2000 ]
then
    if sudo grep -q -i -E 'attack|malware|risk|corrupted|malicious|dangerous' $file_path || [[ $(file -bi $file_path) != *charset=us-ascii* ]];
    then
        if [ ! -d "$safe_path" ]; then
            mkdir -p $safe_path
        fi
        mv $file_path $safe_path
        echo $filename
        exit 1
    fi
fi

echo "SAFE"
exit 0