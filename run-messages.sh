#!/bin/sh

make && cd data && sudo make install && cd .. && DISPLAY=:1 ELM_SCALE=2 ELM_FINGER_SIZE=80 ELM_THEME=gry mokomessages/mokomessages
