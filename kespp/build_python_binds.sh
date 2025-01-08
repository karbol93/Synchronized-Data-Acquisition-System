#!/bin/bash

mkdir -p build
echo "Building for CPython 3.6"
g++ -shared -fPIC -std=c++17 -Isrc/include $(venv36/bin/python -m pybind11 --includes) src/kespp_py.cpp -o build/kespp$(python3.6-config --extension-suffix)
echo "Building for CPython 3.7"
g++ -shared -fPIC -std=c++17 -Isrc/include $(venv37/bin/python -m pybind11 --includes) src/kespp_py.cpp -o build/kespp$(python3.7-config --extension-suffix)
echo "Building for CPython 3.8"
g++ -shared -fPIC -std=c++17 -Isrc/include $(venv38/bin/python -m pybind11 --includes) src/kespp_py.cpp -o build/kespp$(python3.8-config --extension-suffix)
echo "Building for CPython 3.9"
g++ -shared -fPIC -std=c++17 -Isrc/include $(venv39/bin/python -m pybind11 --includes) src/kespp_py.cpp -o build/kespp$(python3.9-config --extension-suffix)
echo "Building for CPython 3.10"
g++ -shared -fPIC -std=c++17 -Isrc/include $(venv310/bin/python -m pybind11 --includes) src/kespp_py.cpp -o build/kespp$(python3.10-config --extension-suffix)
echo "Building for CPython 3.11"
g++ -shared -fPIC -std=c++17 -Isrc/include $(venv311/bin/python -m pybind11 --includes) src/kespp_py.cpp -o build/kespp$(python3.11-config --extension-suffix)
