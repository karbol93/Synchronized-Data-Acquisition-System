#!/bin/bash

mkdir -p build/kespp_python
echo "Building CPython 3.6"
g++ -shared -fPIC -std=c++17 -Iinclude -I/usr/include/python3.6m -I"venv36/lib/python3.6/site-packages/pybind11/include" src/kespp_py.cpp -o build/kespp_python/kespp`python3.6-config --extension-suffix`
echo "Building CPython 3.7"
g++ -shared -fPIC -std=c++17 -Iinclude -I/usr/include/python3.7m -I"venv37/lib/python3.7/site-packages/pybind11/include" src/kespp_py.cpp -o build/kespp_python/kespp`python3.7-config --extension-suffix`
echo "Building CPython 3.8"
g++ -shared -fPIC -std=c++17 -Iinclude -I/usr/include/python3.8 -I"venv38/lib/python3.8/site-packages/pybind11/include" src/kespp_py.cpp -o build/kespp_python/kespp`python3.8-config --extension-suffix`
echo "Building CPython 3.9"
g++ -shared -fPIC -std=c++17 -Iinclude -I/usr/include/python3.9 -I"venv39/lib/python3.9/site-packages/pybind11/include" src/kespp_py.cpp -o build/kespp_python/kespp`python3.9-config --extension-suffix`
echo "Building CPython 3.10"
g++ -shared -fPIC -std=c++17 -Iinclude -I/usr/include/python3.10 -I"venv310/lib/python3.10/site-packages/pybind11/include" src/kespp_py.cpp -o build/kespp_python/kespp`python3.10-config --extension-suffix`
echo "Building CPython 3.11"
g++ -shared -fPIC -std=c++17 -Iinclude -I/usr/include/python3.11 -I"venv311/lib/python3.11/site-packages/pybind11/include" src/kespp_py.cpp -o build/kespp_python/kespp`python3.11-config --extension-suffix`
