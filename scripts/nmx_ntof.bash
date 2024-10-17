#!/bin/bash

BROKER=${DAQLITE_BROKER:-}

TOPIC="-t nmx_detector"

../build/bin/daqlite $BROKER $TOPIC -f ../configs/nmx/nmx.json &
../build/bin/daqlite $BROKER $TOPIC -f ../configs/nmx/nmxtof.json &
../build/bin/daqlite $BROKER $TOPIC -f ../configs/nmx/nmx2d_tof.json &
../build/bin/daqlite $BROKER $TOPIC -f ../configs/nmx/nmxtof2d.json


