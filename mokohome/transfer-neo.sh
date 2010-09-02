#!/bin/sh

./build-arm.sh $* && scp build/mokohome root@neo:/home/root/tmp/mokosuite/
