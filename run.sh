#!/bin/bash

git push
qmake
make
cp config.ini.default /bin/config.ini
cd bin
./OTRS &
