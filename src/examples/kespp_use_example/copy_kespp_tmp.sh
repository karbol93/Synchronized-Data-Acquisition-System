#!/bin/bash

if [[ $BASH_SOURCE == */* ]]; then
    cd -- "${BASH_SOURCE%/*}/"
fi

cp -r ../../kespp_python .
cp -r ../../../build/kespp_python/* kespp_python/
rm kespp_python/copy_binaries_tmp.sh
