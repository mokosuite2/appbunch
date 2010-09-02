#!/bin/sh

cd libmokosuite && ./build-arm.sh && cd .. &&
for i in moko*/; do
	cd "$i" && ./build-arm.sh && cd ..
done
