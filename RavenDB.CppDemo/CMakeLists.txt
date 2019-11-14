# CMakeList.txt : CMake project for RavenDB.CppDemo, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.13)
set(CMAKE_CXX_STANDARD 17)

add_executable (RavenDB.CppDemo "RavenDB.CppDemo.cpp" "RavenDB.CppDemo.h")
