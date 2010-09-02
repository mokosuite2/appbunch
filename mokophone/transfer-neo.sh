#!/bin/sh

./build-arm.sh $* && scp build/mokophone root@neo:/home/root/tmp/mokosuite/
