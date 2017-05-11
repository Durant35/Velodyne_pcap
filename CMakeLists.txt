cmake_minimum_required(VERSION 2.8)

project(Velodyne_pcap)

add_executable(Velodyne_pcap Velodyne_pcap.cpp)
target_link_libraries(Velodyne_pcap pcap)

