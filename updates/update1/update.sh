#!/bin/bash
d=$(dirname $0)
echo 'Введи пароль "cern" ;D'
mysql -u root -p tt < ${d}/usrConf.sql
mysql -u conf tt < ${d}/defaults.sql