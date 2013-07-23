cmake_minimum_required (VERSION 2.8.0)
project (lgdpj)

# config project attributes
set (PROJECT_VERSION 0.0.1)

# set cmake modules path
set (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# config output directories
set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# config source directories
set (SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
set (TEST_DIR ${PROJECT_SOURCE_DIR}/test)
set (THIRDPARTY_DIR ${PROJECT_SOURCE_DIR}/thirdparty)

# config data directories
set (DATA_DIR ${PROJECT_SOURCE_DIR}/data)

set (CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-g")

# compile shipped libraries
# add_subdirectory (thirdparty)

# compile source
set (TOOLS_DIR ${PROJECT_SOURCE_DIR}/tools)

add_subdirectory (thirdparty)
add_subdirectory (src)
