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

# Not used with daqlite deployment - uncomment if needed
$DAQLITE_HOME/bin/daqlite $BROKER -f $DAQLITE_CONFIG/dream/dream_bwe.json $KAFKA_CONFIG &
$DAQLITE_HOME/bin/daqlite $BROKER -f $DAQLITE_CONFIG/dream/dream_fwe.json $KAFKA_CONFIG &
$DAQLITE_HOME/bin/daqlite $BROKER -f $DAQLITE_CONFIG/dream/dream_mantle.json $KAFKA_CONFIG &
$DAQLITE_HOME/bin/daqlite $BROKER -f $DAQLITE_CONFIG/dream/dream_hr.json $KAFKA_CONFIG &
$DAQLITE_HOME/bin/daqlite $BROKER -f $DAQLITE_CONFIG/dream/dream_sans.json $KAFKA_CONFIG &
$DAQLITE_HOME/bin/daqlite $BROKER -f $DAQLITE_CONFIG/dream/dream_cbm.json $KAFKA_CONFIG &
