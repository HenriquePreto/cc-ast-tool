#!/bin/bash

SCRIPT=$(realpath "$0")
SCRIPT_PATH=$(dirname "$SCRIPT")
${SCRIPT_PATH}/cc_tool.sh $@
clang $@