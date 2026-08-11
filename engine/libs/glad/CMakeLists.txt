cmake_minimum_required(VERSION 3.20)
project(glad C)

# Collect all header and source files from GLAD directory
file(GLOB_RECURSE GLAD_HEADERS 
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)
file(GLOB_RECURSE GLAD_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

# Declare GLAD as a static library
add_library(glad STATIC ${GLAD_SOURCES} ${GLAD_HEADERS})

# Expose include directories to dependent targets (e.g., your engine)
target_include_directories(glad 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Match C standard
target_compile_features(glad PUBLIC c_std_99)
