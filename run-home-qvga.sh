#!/bin/bash

DISPLAY=":1"
if [ "$1" != "" ]; then
DISPLAY="$1"
fi

make &&
cd data && sudo make install && cd .. &&
sudo DISPLAY=$DISPLAY ELM_FINGER=40 ELM_SCALE=1 ELM_THEME=gry mokohome/mokohome
