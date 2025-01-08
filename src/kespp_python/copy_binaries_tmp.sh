#!/bin/bash

if [[ $BASH_SOURCE == */* ]]; then
    cd -- "${BASH_SOURCE%/*}/"
fi

cp ../../build/kespp_python/* .
