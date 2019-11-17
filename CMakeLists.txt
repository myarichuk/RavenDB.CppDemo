# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.13)
project ("RavenDB.CppDemo")
set(CMAKE_CXX_STANDARD 17)

set (RAVENCPP_SRC "${PROJECT_SOURCE_DIR}/libs/RavenDB" CACHE INTERNAL "RAVENCPP_SRC")
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
  set(BUILD_TRYOUTS OFF)
  set(BUILD_TESTS OFF)
endif()

add_subdirectory("${RAVENCPP_SRC}/repository/")
add_subdirectory ("RavenDB.CppDemo")

message("Raven.CppDemo include: ${RAVENCPP_SRC}/repository/Raven.CppClient")
message("CMAKE_INCLUDE_PATH : ${CMAKE_INCLUDE_PATH}")
message("CMAKE_LIBRARY_PATH : ${CMAKE_LIBRARY_PATH}")
 
link_directories(${CMAKE_LIBRARY_PATH})
include_directories(${RAVENCPP_SRC}/repository/Raven.CppClient ${CMAKE_INCLUDE_PATH})

add_dependencies(RavenDB.CppDemo Raven_CppClient_static)
link_libraries(Raven_CppClient_static)



