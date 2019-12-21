#!/bin/bash

set -e # Exit on any error

# Verify compilation and unit tests for CI environment
platformio test -e downlink_parser_ci -v

# Check for memory mismanagement
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose .pio/build/downlink_parser_ci/program

# Compile and run functional test for environments
platformio run -e downlink_parser_ci
platformio run -e downlink_parser
python -m unittest test.test_downlink_parser