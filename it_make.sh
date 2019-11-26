#!/bin/sh
# -*- coding:utf-8-*-
# Author: hf

node_make()
{
cd  /home/adminuser/usb/zosRelease/zos-core
git checkout .
git pull --rebase
sleep 1
git pull
sleep 2
}


case $1 in
start)
    node_make
;;
stop)
    printf "waiting for this function"
;;
restart)
    printf "waiting for this function"
;;
*)
    printf "Usage: /home/zosroot/zosrun/sh it_nodestart.sh {start|stop|restart}\n"
esac

