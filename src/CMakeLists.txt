# Add executable for openvq
file (GLOB_RECURSE openvq_SOURCES *.cpp)
file (GLOB_RECURSE openvq_HEADERS *.h)
add_executable (openvq ${openvq_HEADERS} ${openvq_SOURCES})

# External dependencies
# Custom FFmpeg paths for x265 support
include_directories(/home/vagrant/ffmpeg_build/include)
link_directories(/home/vagrant/ffmpeg_build/lib)

#set(openvq_DEPS ${LIBAVFORMAT_LIBRARY} ${LIBAVCODEC_LIBRARY} ${LIBAVUTIL_LIBRARY} ${LIBSWSCALE_LIBRARY} ${OpenCV_LIBS} ${CMAKE_THREAD_LIBS_INIT})
set(openvq_DEPS ${OpenCV_LIBS} ${CMAKE_THREAD_LIBS_INIT})
if (NOT WIN32)
    list (APPEND openvq_DEPS ${Boost_PROGRAM_OPTIONS_LIBRARY_DEBUG})
endif ()
if (LIBZ)
  list (APPEND openvq_DEPS ${LIBZ})
endif ()

set (FFMPEG_LIB_DIR /home/vagrant/ffmpeg_build/lib)

target_link_libraries(openvq avutil avcodec avformat swscale ${openvq_DEPS})
#target_link_libraries(openvq ${FFMPEG_LIB_DIR}/libavutil.a ${FFMPEG_LIB_DIR}/libavcodec.a ${FFMPEG_LIB_DIR}/libavformat.a ${FFMPEG_LIB_DIR}/libswscale.a ${openvq_DEPS})
#target_link_libraries (openvq ${openvq_DEPS})

# Install target
install (TARGETS openvq DESTINATION bin)
