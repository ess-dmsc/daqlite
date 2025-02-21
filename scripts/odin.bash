#!/bin/bash

BROKER=${DAQLITE_BROKER:-}

KAFKA_CONFIG=""

# Check if we are in production environment and set KAFKA_CONFIG accordingly
# or set dev environment variables
if [ "$DAQLITE_PRODUCTION" = "true" ]; then
    KAFKA_CONFIG="-k $DAQLITE_CONFIG/kafka-config-daqlite.json"
else
    DAQLITE_HOME="../build"
    DAQLITE_CONFIG="../configs"
fi

$DAQLITE_HOME/bin/daqlite $BROKER -f $DAQLITE_CONFIG/odin/odin.json $KAFKA_CONFIG &
$DAQLITE_HOME/bin/daqlite $BROKER -f $DAQLITE_CONFIG/odin/odin_next.json $KAFKA_CONFIG &
