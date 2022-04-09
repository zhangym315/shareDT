#### Test cases
# While adding a new case, don't forgot to update three part
#   add_executable
#   target_link_libraries
#   set(TESTING_CASES...)
####

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/googletest/include
                    ${FFMPEG_INCLUDE_DIR}
                    ${CMAKE_SOURCE_LIBVNC}
                    ${CMAKE_SOURCE_LIBVNC}/libvncserver
                    ${CMAKE_SOURCE_LIBVNC}/common
                    ${CMAKE_SOURCE_LIBVNC}/ffmpeg
                    ${LIBSOURCE_FFMPEG_DIR}
                    )

set(TESTINGCASE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src)

#### Testing case started

add_executable(ffmpeg_encode_decode
                ${TESTINGCASE_SOURCE}/FfmpegEncodeDecode.cpp
                ${SHAREDT_SRC}/ffmpeg/FfmpegUtil.cpp
                ${SHAREDT_SRC}/util/TimeUtil.c
               )

target_link_libraries(ffmpeg_encode_decode vncserver vncclient
                      gtest gtest_main ${FFMPEG_LIBRARIES}
                      ${PNG_LIBRARIES}
                      ${FFMPEG_REQUIRED_LIBS}
                      ${LZMA_LIBRARIES}
                      ${BZIP2_LIBRARIES}
                      ${X265_LIBRARIES}
                      ${X264_LIBRARY}
#                      ${${PROJECT_NAME}_PLATFORM_LIBS}
#                      ${X265_LIBRARIES}
                      )

set(TESTING_CASES ffmpeg_encode_decode
        )

