cmake_minimum_required (VERSION 2.6.2)
set (PROJECT_NAME openvq)
project (${PROJECT_NAME} CXX C)

# Set path for module cmake files
set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/mod)

# Include
include (cmake/libs.cmake)

#
# Common project settings
#
set (openvq_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    else()
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    endif()
    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -DOPENVQ_DEBUG")
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fopenmp -Wall -Werror -O3")
    if (Threads_FOUND)
        set (CMAKE_EXE_LINKER_FLAGS "-pthread")
    endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -DOPENVQ_DEBUG")
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Werror -O3")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /openmp")
	set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ox /MT")
	set (CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} /MTd /DOPENVQ_DEBUG")
	add_definitions(-D_USE_MATH_DEFINES)
endif ()
include_directories(${openvq_SOURCE_DIR})

#
# Main project target
#
add_subdirectory (${openvq_SOURCE_DIR})
