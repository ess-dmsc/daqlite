#!/bin/bash

BROKER=${DAQLITE_BROKER:-}

../build/bin/daqlite $BROKER -f ../configs/heimdal/heimdal.json
