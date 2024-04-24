#!/bin/bash

BROKER=${DAQLITE_BROKER:-}
KAFKA_CONFIG=${DAQLITE_KAFKA_CONFIG:-}



../build/bin/daqlite $BROKER -f ../configs/odin/odin.json -k $KAFKA_CONFIG &
../build/bin/daqlite $BROKER -f ../configs/odin/odintof.json -k $KAFKA_CONFIG
