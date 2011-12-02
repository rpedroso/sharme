cmake_minimum_required (VERSION 2.6)
project (sharme)

# The version number.
set (sharme_VERSION_MAJOR 0)
set (sharme_VERSION_MINOR 3)
set (sharme_LABEL_HEADER "\"sharme session ${sharme_VERSION_MAJOR}.${sharme_VERSION_MINOR}\"")
 
set (sharme_PORT "\"8000\"")
# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/src/sharme_config.h.in"
  "${PROJECT_BINARY_DIR}/src/sharme_config.h"
  )

if(WINDOWS)
    add_definitions("-DMSWINDOWS")
    add_definitions("-DDEBUG")
    add_definitions("-D__MSW__")
endif(WINDOWS)

if(UNIX)
    add_definitions("-D__XLIB__")
    #add_definitions("-I/home/rpedroso/Downloads/fltk-1.3.0")
    #add_definitions("-D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D_THREAD_SAFE -D_REENTRANT")
    #add_definitions("-lXext -lpthread -ldl -lm -lX11")
endif(UNIX)

#find_package(JPEG REQUIRED)
#include_directories(${JPEG_INCLUDE_DIRS})
#set(LIBS ${LIBS} ${JPEG_LIBRARIES})

add_definitions("-O3")
#add_definitions("-g")
add_definitions("-ffast-math")

# add the binary tree to the search path for include files
include_directories("${PROJECT_BINARY_DIR}/src")
include_directories ("${PROJECT_SOURCE_DIR}/lib")
include_directories ("${PROJECT_SOURCE_DIR}/include")

add_subdirectory (lib) 
add_subdirectory (src) 

