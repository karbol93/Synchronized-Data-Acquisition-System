#!/bin/bash

if [[ $BASH_SOURCE == */* ]]; then
    cd -- "${BASH_SOURCE%/*}/"
fi

git ls-files --stage | grep '.sh$' | cut -f2 | xargs -d '\n' git update-index --chmod=+x
