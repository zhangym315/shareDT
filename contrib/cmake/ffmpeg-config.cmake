set(VERSION 4.3.1)
set(FFMPEG_INSTALL ${CMAKE_SOURCE_CONTRIB}/ffmpeg/install/)
set(FFMPEG_INCLUDE_DIR ${FFMPEG_INSTALL}/include/)
set(FFMPEG_LIBRARIES ${FFMPEG_INSTALL}/lib/libavcodec.a
                     ${FFMPEG_INSTALL}/lib/libavfilter.a
                     ${FFMPEG_INSTALL}/lib/libavutil.a
                     ${FFMPEG_INSTALL}/lib/libswscale.a
                     ${FFMPEG_INSTALL}/lib/libavdevice.a
                     ${FFMPEG_INSTALL}/lib/libavformat.a
                     ${FFMPEG_INSTALL}/lib/libswresample.a
        )

message(STATUS "Found FFMPEG at ${FFMPEG_INSTALL}")
set(FFMPEG_FOUND TRUE)
