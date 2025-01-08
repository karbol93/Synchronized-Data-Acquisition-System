#!/bin/bash

if [[ $BASH_SOURCE == */* ]]; then
    cd -- "${BASH_SOURCE%/*}/"
fi

g++-10 kespp_use_example.cpp -Wall -pedantic -std=c++17 -pthread -I../../../include -o ../../../build/kespp_use_example
# g++-10 kespp_use_example.cpp -Wall -pedantic -std=c++17 -pthread -ggdb3 -o ../../../build/kespp_use_example
