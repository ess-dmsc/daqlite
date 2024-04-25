#!/bin/bash

# export DAQLITE_BROKER='10.100.1.21'
export DAQLITE_KAFKA_CONFIG='/workspaces/daqlite/configs/kafka_config.json'

if [ -f /tmp/sshuttle.pid ]; then
    pid=$(cat /tmp/sshuttle.pid)
    kill $pid
fi

sshuttle -D -r ssh4.esss.dk 10.100.0.0/16 --pidfile /tmp/sshuttle.pid

/workspaces/daqlite/scripts/odin.bash