#!/bin/sh

./build-arm.sh $* && scp build/libmokosuite.so.0.0.0 root@neo:/home/root/tmp/mokosuite/
