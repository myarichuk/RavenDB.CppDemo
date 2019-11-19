# CMakeList.txt : CMake project for RavenDB.CppDemo, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.13)
project("RavenDB.CppDemo.Executable")
set(CMAKE_CXX_STANDARD 17)

find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)
find_package(Threads REQUIRED)

add_executable (RavenDB.CppDemo "RavenDB.CppDemo.cpp" "RavenDB.CppDemo.h")
target_include_directories(RavenDB.CppDemo PUBLIC ${CMAKE_INCLUDE_PATH})

target_link_libraries(RavenDB.CppDemo #"${RAVENCPP_SRC}/binary/Raven.CppClient/Raven.CppClient.lib"  
	Raven_CppClient 
    OpenSSL::SSL 
    OpenSSL::Crypto 
    ${CURL_LIBRARIES} 
    Threads::Threads)