#!/bin/sh

REPO=/home/daniele/repository/openmoko

$REPO/misc/xephyr &
sleep 2
DISPLAY=:1 $REPO/misc/matchbox-window-manager-2-openmoko &
