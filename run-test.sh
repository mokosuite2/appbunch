#!/bin/bash

DISPLAY=":1"
if [ "$2" != "" ]; then
DISPLAY="$2"
fi

make &&
sudo DISPLAY=$DISPLAY ELM_FINGER=80 ELM_SCALE=2 ELM_THEME=$PWD/../elementary-theme-android/android.edj "tests/$1"
