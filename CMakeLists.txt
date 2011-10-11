cmake_minimum_required (VERSION 2.6)
project (sharme)

# The version number.
set (client_VERSION_MAJOR 1)
set (client_VERSION_MINOR 0)
 
if(WINDOWS)
add_definitions("-DMSWINDOWS")
add_definitions("-DDEBUG")
add_definitions("-D__MSW__")
endif(WINDOWS)

if(LINUX)
  add_definitions("-D__XLIB__")
endif(LINUX)

#find_package(JPEG REQUIRED)
#include_directories(${JPEG_INCLUDE_DIRS})
#set(LIBS ${LIBS} ${JPEG_LIBRARIES})

add_definitions("-O3")
add_definitions("-ffast-math")

# configure a header file to pass some of the CMake settings
# to the source code
#configure_file (
#  "${PROJECT_SOURCE_DIR}/TutorialConfig.h.in"
#  "${PROJECT_BINARY_DIR}/TutorialConfig.h"
#  )
find_package (Threads)
 
# add the binary tree to the search path for include files
include_directories("${PROJECT_BINARY_DIR}")
include_directories ("${PROJECT_SOURCE_DIR}/lib")
include_directories ("${PROJECT_SOURCE_DIR}/include")

add_subdirectory (lib) 
add_subdirectory (src) 

