#/bin/bash
d=$(dirname $0)
mkdir -p ${d}/old
cp /etc/httpd/conf/vhosts/tt ${d}/old/tt
rm /etc/httpd/conf/vhosts/tt
cp /etc/httpd/conf/httpd.conf ${d}/old/httpd.conf
cp ${d}/SheduleSrv /etc/httpd/conf/vhosts/SheduleSrv
cat ${d}/httpd.conf > /etc/httpd/conf/httpd.conf
systemctl restart httpd
