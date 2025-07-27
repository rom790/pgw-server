#!/bin/bash
SERVER_PID=""  
BUILD_DIR="build"

if [ ! -d "$BUILD_DIR" ]; then
    echo "Error: Build directory $BUILD_DIR does not exist"
    exit 1
fi

build/bin/unit_tests

build/bin/integration_tests


build/bin/pgw_server "configs/load_test_server_config.json" &
SERVER_PID=$!
sleep 2

build/bin/mass_test "configs/load_test_client_config.json" 100 0


echo "Stopping server..."
kill -TERM "$SERVER_PID" || kill -9 "$SERVER_PID"
wait "$SERVER_PID" 2>/dev/null

exit 0