#!/bin/bash

BROKER=${DAQLITE_BROKER:-}

../build/bin/daqlite $BROKER -f ../configs/cbm/ibm_tof.json &
