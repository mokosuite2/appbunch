#!/bin/bash

function startproc() {
EXEC="mokosettings/mokosettings"
if [ "$1" == "debug" ]; then
EXEC="LD_LIBRARY_PATH=libmokosuite/.libs gdb mokosettings/.libs/mokosettings"
fi

echo $EXEC
sudo DISPLAY=:1 ELM_FINGER=80 ELM_SCALE=2 ELM_THEME=android DBUS_SYSTEM_BUS_ADDRESS="tcp:host=neo,port=8000" $EXEC
}

make &&
cd data && sudo make install && cd .. &&
startproc $1
