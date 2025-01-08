#!/bin/bash

sudo apt update
sudo apt install software-properties-common -y
sudo add-apt-repository ppa:deadsnakes/ppa -y

sudo apt install python3.6 python3.6-dev python3.6-venv -y
sudo apt install python3.7 python3.7-dev python3.7-venv -y
sudo apt install python3.8 python3.8-dev python3.8-venv -y
sudo apt install python3.9 python3.9-dev python3.9-venv -y
sudo apt install python3.10 python3.10-dev python3.10-venv -y
sudo apt install python3.11 python3.11-dev python3.11-venv -y

./create_venvs.sh
