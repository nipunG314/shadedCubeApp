#!/bin/bash
cd glfw-3.3.2
cmake .
sudo make
sudo make install
cd ../..

sudo apt-get install libglm-dev

dir="$(dirname "$0")"
"$(source "$dir"/env.sh)"

