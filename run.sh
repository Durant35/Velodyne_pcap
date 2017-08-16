#!/bin/bash

if [ $# -lt 1 ] ; then
    echo "parameters error, you should use like this: ./run.sh <option>"
    echo "<option> must be one of following"
    echo "   setup"
    echo "   setup32"
    echo "   capture <lidar-type> save-name"
    echo "      <lidar-type> can be VLP16, HDL32, HDL64"
    echo "   clean"
    exit -1
fi

if [ $1 == "setup" ] ; then
    rm -rf build Velodyne_pcap
    echo "--[BASH INFO] generated files cleaned..."

    mkdir build && cd build

    cmake ..
    if [ $? -gt 0 ]; then
        echo ""
        echo ""
        echo "--[BASH INFO] dependencies miss"
        echo "  ------- pcap ------ "
        echo "  <1> sudo apt-get install libpcap0.8-dev"
        exit -1
    fi
    make
    if [ $? -gt 0 ]; then
        echo ""
        echo ""
        echo "--[BASH INFO] dependencies miss"
        echo "  ------- pcap ------ "
        echo "  <1> sudo apt-get install libpcap0.8-dev"
        exit -1
    fi
    cp Velodyne_pcap ..
    echo "--[BASH INFO] Velodyne_pcap generated: ../Velodyne_pcap"

elif [ $1 == "setup32" ]; then
    rm -rf build Velodyne_pcap
    echo "--[BASH INFO] generated files cleaned..."

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
    cp Velodyne_pcap ..
    echo "--[BASH INFO] 32-bit Velodyne_pcap generated: ../Velodyne_pcap"

elif [ $1 == "capture" ]; then
    if [ $# -lt 2 ] ; then
        echo "parameters error, you should use like this: "
        echo " ./run.sh capture <lidar-type> [save-name]"
        echo "   <lidar-type> can be VLP16, HDL32, HDL64"
        exit -1
    fi

    sudo ./Velodyne_pcap $2 $3
    if [ $? -gt 0 ]; then
        echo "--[BASH INFO] run './run.sh setup|setup32' to build the project first"
        exit -1
    fi

elif [ $1 == "clean" ] ; then
    rm -rf build Velodyne_pcap
    echo "--[BASH INFO] generated files cleaned..."

else
    echo "parameters error, you should use like this: ./run.sh <option>"
    echo "<option> must be one of following"
    echo "   setup"
    echo "   setup32"
    echo "   capture <lidar-type> save-name"
    echo "      <lidar-type> can be VLP16, HDL32, HDL64"
    echo "   clean"
fi
