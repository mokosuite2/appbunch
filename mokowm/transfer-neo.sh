#!/bin/sh

./build-arm.sh $* && scp build/mokowm root@neo:/home/root/tmp/mokosuite/
