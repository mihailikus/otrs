#!/bin/bash

qmake
make
cp config.ini.default /bin/config.ini
./bin/OTRS &

