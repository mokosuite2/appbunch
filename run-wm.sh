#!/bin/bash

DISPLAY=":1"
if [ "$1" != "" ]; then
DISPLAY="$1"
fi

make &&
cd data && sudo make install && cd .. &&
DISPLAY=$DISPLAY mokowm/mokowm
