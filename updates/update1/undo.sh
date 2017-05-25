#!/bin/bash
d=$(dirname $0)
mysql -u conf tt < ${d}/rmDefaults.sql
echo 'Введи пароль "cern" ;D'
mysql -u root -p tt < ${d}/rmUsrConf.sql