#!/bin/bash
if [[ $EUID -ne 0 ]]; then
    echo "Этот скрипт нужно запускать с правами суперпользователя!"
    echo "Попробуй sudo $0"  
    exit 1
fi
mkdir -p installed-updates
declare -a newUpdates=`ls updates`
declare -a oldUpdates=`ls installed-updates`

for update in "${newUpdates[@]}"
do
    if [[ ${oldUpdates[$update]} ]]
    then
        continue
    fi
    echo "установка $update"
    "./updates/$update/update.sh"
    cp -R "updates/$update" "installed-updates"
done