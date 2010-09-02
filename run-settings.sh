#!/bin/bash

function startproc() {
EXEC="mokosettings/mokosettings"
if [ "$1" == "debug" ]; then
EXEC="LD_LIBRARY_PATH=libmokosuite/.libs gdb mokosettings/.libs/mokosettings"
fi

DISPLAY=":1"
if [ "$2" != "" ]; then
DISPLAY="$2"
fi

echo $EXEC
sudo DISPLAY=$DISPLAY ELM_FINGER=80 ELM_SCALE=2 ELM_THEME=gry DBUS_SYSTEM_BUS_ADDRESS="tcp:host=neo,port=8000" $EXEC
}

make &&
cd data && sudo make install && cd .. &&
startproc $1 $2
#sudo DISPLAY=:1 ELM_FINGER=80 ELM_SCALE=2 ELM_THEME=gry DBUS_SYSTEM_BUS_ADDRESS="tcp:host=neo,port=8000" $DEBUG mokosettings/mokosettings
