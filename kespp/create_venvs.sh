#!/bin/bash

deactivate

python3.6 -m venv venv36
. venv36/bin/activate
pip install --upgrade pip
pip install -r requirements.txt
deactivate

python3.7 -m venv venv37
. venv37/bin/activate
pip install --upgrade pip
pip install -r requirements.txt
deactivate

python3.8 -m venv venv38
. venv38/bin/activate
pip install --upgrade pip
pip install -r requirements.txt
deactivate

python3.9 -m venv venv39
. venv39/bin/activate
pip install --upgrade pip
pip install -r requirements.txt
deactivate

python3.10 -m venv venv310
. venv310/bin/activate
pip install --upgrade pip
pip install -r requirements.txt
deactivate

python3.11 -m venv venv311
. venv311/bin/activate
pip install --upgrade pip
pip install -r requirements.txt
deactivate
