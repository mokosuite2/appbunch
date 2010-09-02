#!/bin/sh

cd libmokosuite && ./transfer-neo.sh && cd .. &&
for i in moko*/; do
	cd "$i" && ./transfer-neo.sh && cd ..
done
