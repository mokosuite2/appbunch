#!/bin/sh

./build-arm.sh $* && scp build/mokosettings root@neo:/home/root/tmp/mokosuite/
