cmake_minimum_required(VERSION 3.15)
project(PackageTest CXX)

find_package(Data_Consumer_Adapter_Interface CONFIG REQUIRED)

add_executable(example example.cpp)
target_link_libraries(example Data_Consumer_Adapter_Interface::Data_Consumer_Adapter_Interface)
