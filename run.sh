#!/bin/bash

git pull
qmake
make
cp -n config.ini.default /bin/config.ini
cd bin
./OTRS &
