#!/bin/bash
if [[ $EUID -ne 0 ]]; then
    echo "Этот скрипт нужно запускать с правами суперпользователя!"
    echo "Попробуй sudo $0"  
    exit 1
fi
declare -a oldUpdates=`ls installed-updates -r`

for update in ${oldUpdates[@]}
do
    echo "откат $update"
    "./installed-updates/$update/undo.sh"
    rm -rf "installed-updates/$update"
done