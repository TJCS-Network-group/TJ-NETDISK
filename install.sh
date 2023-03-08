#!/bin/bash

ori_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd $ori_DIR"/source/server";make clean;make
rm -rf ../pool
mkdir ../pool

cd $ori_DIR;

cd $ori_DIR"/source/web";
/bin/cp -r * /var/www
#webÎÄ¼þÒÆ¶¯£¿
cd $ori_DIR;

cd $ori_DIR"/source/sql";
mysql -u root -p < netdisk.db.sql;



