#!/bin/bash
./install_minimal_env.sh

sudo add-apt-repository universe -y
sudo apt install doxygen -y # doxygen-gui doxygen-doc graphviz

sudo apt install v4l-utils -y
