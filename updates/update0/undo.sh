#/bin/bash
d=$(dirname $0)
rm /etc/httpd/conf/vhosts/SheduleSrv
cp ${d}/old/tt /etc/httpd/conf/vhosts/tt
cat ${d}/old/httpd.conf > /etc/httpd/conf/httpd.conf
systemctl restart httpd
