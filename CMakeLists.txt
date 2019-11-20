# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.13)
project ("RavenDB.CppDemo")
set(CMAKE_CXX_STANDARD 17)

#do required configuration for compiling this in Windows 
if(MSVC)
	set(CMAKE_CXX_FLAGS_DEBUG "/MTd /Zi /Ob0 /Od /RTC1")
	set(CMAKE_CXX_FLAGS_RELEASE "/MT /O2 /Ob2 /DNDEBUG")	
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
 
set (RAVENCPP_SRC "${PROJECT_SOURCE_DIR}/libs/RavenDB" CACHE INTERNAL "RAVENCPP_SRC")

#fetch RavenDB client from a repo
include(FetchContent)
FetchContent_Declare(
  ravendb_client_cpp
  GIT_REPOSITORY https://github.com/myarichuk/ravendb-cpp-client.git
  GIT_TAG        master
  SOURCE_DIR ${RAVENCPP_SRC}/repository SUBBUILD_DIR ${RAVENCPP_SRC}/subbuild  BINARY_DIR ${RAVENCPP_SRC}/binary
  )

FetchContent_GetProperties(ravendb_client_cpp)
if(NOT ravendb_client_cpp_POPULATED)  
  FetchContent_Populate(ravendb_client_cpp)

  #since we want only the client, we don't want to compile tests and tryouts, only the client
  set(BUILD_TRYOUTS OFF)
  set(BUILD_TESTS OFF)
  add_subdirectory("${RAVENCPP_SRC}/repository/" "${RAVENCPP_SRC}/binary")
endif() 

list(APPEND CMAKE_INCLUDE_PATH ${RAVENCPP_SRC}/repository/Raven.CppClient)
set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} CACHE INTERNAL "CMAKE_INCLUDE_PATH")

message("Raven.CppDemo include: ${RAVENCPP_SRC}/repository/Raven.CppClient")
message("CMAKE_INCLUDE_PATH : ${CMAKE_INCLUDE_PATH}")
message("CMAKE_LIBRARY_PATH : ${CMAKE_LIBRARY_PATH}")

include_directories(${CMAKE_INCLUDE_PATH})

add_subdirectory ("RavenDB.CppDemo")

link_directories(${CMAKE_LIBRARY_PATH})

add_dependencies(RavenDB.CppDemo Raven_CppClient_static)
link_libraries(Raven_CppClient_static)



