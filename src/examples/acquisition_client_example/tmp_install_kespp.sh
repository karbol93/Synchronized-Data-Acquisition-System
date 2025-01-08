#!/bin/bash

if [[ $BASH_SOURCE == */* ]]; then
    cd -- "${BASH_SOURCE%/*}/"
fi

python3.11 -m venv venv
. venv/bin/activate
pip install -r requirements.txt
deactivate

cp -r ../../kespp_python .
cp -r ../../../build/kespp_python/* kespp_python/
rm kespp_python/copy_binaries_tmp.sh
