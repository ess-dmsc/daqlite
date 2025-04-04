#!/bin/bash

BROKER=${DAQLITE_BROKER:-}

KAFKA_CONFIG=""
CALIBRATION=""

# Check if we are in production environment and set KAFKA_CONFIG accordingly
# or set dev environment variables
if [ "$DAQLITE_PRODUCTION" = "true" ]; then
    KAFKA_CONFIG="-k $DAQLITE_CONFIG/kafka-config-daqlite.json"
else
    DAQLITE_HOME="../build"
    DAQLITE_CONFIG="../configs"
fi

$DAQLITE_HOME/bin/fylgje $BROKER -f $DAQLITE_CONFIG/bifrost/fylgje.json -c $DAQLITE_CONFIG/bifrost/calibration.json $KAFKA_CONFIG