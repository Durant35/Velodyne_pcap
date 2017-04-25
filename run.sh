#!/bin/bash

if [ $# != 1 ] ; then
    echo "parameters error, you should use like this: ./run.sh <option>"
    echo "<option> can be setup, capture, clean"
    exit -1
fi

if [ $1 == "setup" ] ; then
    rm -rf build
    mkdir build && cd build
    cmake -DCMAKE_CXX_FLAGS=-m32 -DCMAKE_SHARED_LINKER_FLAGS=-m32 ..
    if [ $? -gt 0 ]; then
        echo ""
        echo ""
        echo "--[BASH INFO] dependencies miss"
        echo "  ------- 32-bit standard libary ------ "
        echo "  sudo dpkg --add-architecture i386" 
        echo "  sudo apt-get update" 
        echo "  <1> sudo apt-get install g++-multilib" 
        echo "  <2> sudo apt-get install lib32stdc++6"
        echo "  ------- pcap ------ "
        echo "  <1> sudo apt-get install libpcap0.8-dev:i386"
        exit -1
    fi

    make
    cp Velodyne_pcap ..
elif [ $1 == "capture" ]; then
    sudo ./Velodyne_pcap
elif [ $1 == "clean" ] ; then
    rm -rf build Velodyne_pcap
else
    echo "parameters error, you should use like this: ./run.sh <option>"
    echo "<option> can be setup, run, clean"
    exit -1
fi