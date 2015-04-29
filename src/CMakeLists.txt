# Add executable for openvq
file (GLOB_RECURSE openvq_SOURCES *.cpp)
file (GLOB_RECURSE openvq_HEADERS *.h)
add_executable (openvq ${openvq_HEADERS} ${openvq_SOURCES})

# External dependencies
set(openvq_DEPS ${LIBAVFORMAT_LIBRARY} ${LIBAVCODEC_LIBRARY} ${LIBAVUTIL_LIBRARY} ${LIBSWSCALE_LIBRARY} ${OpenCV_LIBS} ${CMAKE_THREAD_LIBS_INIT})
if (NOT WIN32)
    list (APPEND openvq_DEPS ${Boost_PROGRAM_OPTIONS_LIBRARY_DEBUG})
endif ()
if (LIBZ)
  list (APPEND openvq_DEPS ${LIBZ})
endif ()

target_link_libraries (openvq ${openvq_DEPS})

# Install target
install (TARGETS openvq DESTINATION bin)