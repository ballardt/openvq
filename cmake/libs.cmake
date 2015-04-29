# Find project wide required libraries

# Setup prefixes and suffixes for three main platforms
if (APPLE)
  set (FIND_LIBRARY_PATHS "")
  set (CMAKE_FIND_LIBRARY_PREFIXES "" "lib")
  set (CMAKE_FIND_LIBRARY_SUFFIXES ".dylib" ".a" ".lib" ".so")
elseif (UNIX)
  set (FIND_LIBRARY_PATHS "")
  set (CMAKE_FIND_LIBRARY_PREFIXES "lib")
  set (CMAKE_FIND_LIBRARY_SUFFIXES ".so" ".a")
elseif (WIN32)
  set (FIND_LIBRARY_PATHS "C:/openvq/deps/" "C:/openvq/deps/libav/win64/usr/bin")
  set (CMAKE_PREFIX_PATH "C:/openvq/deps/opencv/build")
  set (CMAKE_FIND_LIBRARY_PREFIXES "")
  set (CMAKE_FIND_LIBRARY_SUFFIXES ".lib" ".dll")
  set (WIN64_LIBAV_DIR C:/openvq/deps/libav/win64/usr)
endif ()

#
# Multi threading
#
find_package(Threads)

#
# libav
#
find_package (LibAVFormat 55.12.0 REQUIRED)
find_package (LibAVCodec 55.34.1 REQUIRED)
find_package (LibAVUtil 53.3.0 REQUIRED)
find_package (LibSWScale 2.1.2 REQUIRED)

# All libav libraries found, add include and library directories
include_directories (${LIBAVFORMAT_INCLUDE_DIRS})
include_directories (${LIBAVCODEC_INCLUDE_DIRS})
include_directories (${LIBAVUTIL_INCLUDE_DIRS})
include_directories (${LIBSWSCALE_INCLUDE_DIRS})

# __STDC_CONSTANT_MACROS is necessary for libav to compile as C++
add_definitions (-D__STDC_CONSTANT_MACROS)


#
# Boost libraries
#
find_package (Boost 1.54 COMPONENTS program_options REQUIRED)
include_directories (${Boost_INCLUDE_DIR})


#
# OpenCV
#
find_package (OpenCV 2.4.8 REQUIRED)
if (NOT OpenCV_FOUND)
    message (FATAL_ERROR "OpenCV 2.4.8 or later not found")
endif ()
message (STATUS "Found OpenCV version ${OpenCV_VERSION}")

# UNIX specific dependencies
if (NOT WIN32)
  find_library (LIBZ z)
  if (NOT LIBZ)
    message (FATAL_ERROR "libz required but not found")
  endif (NOT LIBZ)
endif(NOT WIN32)

# WIN32 specific extra linking hints
if (WIN32)
  link_directories (${LIBAVFORMAT_LIBRARY})
  link_directories (${LIBAVCODEC_LIBRARY})
  link_directories (${LIBAVUTIL_LIBRARY})
  link_directories (${LIBSWSCALE_LIBRARY})
  link_directories (${Boost_LIBRARY_DIR})
endif ()