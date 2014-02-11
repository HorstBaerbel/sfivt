cmake_minimum_required(VERSION 2.6)

project(svift)

#-------------------------------------------------------------------------------
#set up compiler flags and excutable names

if(CMAKE_COMPILER_IS_GNUCXX)
    #check for G++ 4.7+
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE G++_VERSION)
    if (G++_VERSION VERSION_LESS 4.7)
            message(SEND_ERROR "You need at least G++ 4.7 to compile MAMEduino!")
    endif()
    #set up compiler flags for GCC
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2") #support C++11 for std::, optimize
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s") #strip binary
endif()

#finding necessary packages
#-------------------------------------------------------------------------------
find_package(FreeImage REQUIRED)

#-------------------------------------------------------------------------------
#add include directories
set(TARGET_INCLUDE_DIRS
    ${FreeImage_INCLUDE_DIRS}
)

#-------------------------------------------------------------------------------
#define basic sources and headers

set(TARGET_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/framebuffer.h
)

set(TARGET_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/framebuffer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

#-------------------------------------------------------------------------------
#define libraries and directories
set(TARGET_LIBRARIES
    ${FreeImage_LIBRARIES}
)

#-------------------------------------------------------------------------------
#set up build directories

set(dir ${CMAKE_CURRENT_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH ${dir} CACHE PATH "Build directory" FORCE)
set(LIBRARY_OUTPUT_PATH ${dir} CACHE PATH "Build directory" FORCE)

#-------------------------------------------------------------------------------
#define target
include_directories(${TARGET_INCLUDE_DIRS})
add_executable(emulationstation ${TARGET_SOURCES} ${TARGET_HEADERS})
target_link_libraries(emulationstation ${TARGET_LIBRARIES})
