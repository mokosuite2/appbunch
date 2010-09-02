#!/bin/sh

./build-arm.sh $* && scp build/mokopanel root@neo:/home/root/tmp/mokosuite/
