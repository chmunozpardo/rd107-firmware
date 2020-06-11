#!/bin/bash

source ~/.bashrc
source ../esp-idf-v4.0/export.sh

if [ "$1" == "build" ]; then
    idf.py build
fi

if [ "$1" == "flash" ]; then
    if [ $2 == "monitor" ]; then
        idf.py flash monitor
    else
        idf.py flash
    fi
fi

if [ $1 == "monitor" ]; then
    idf.py monitor
fi