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
                    ${VVDEC_INCLUDE_DIR}
                    ${VVENC_INCLUDE_DIR}
                    )

set(TESTINGCASE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src)

#### Testing case started

add_executable(ffmpeg_encode_decode
                ${TESTINGCASE_SOURCE}/FfmpegEncodeDecode.cpp
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


add_executable(vv_encode_decode
        ${TESTINGCASE_SOURCE}/vvEncDecode.cpp
        ${SHAREDT_SERVER_SRC_COMMON}
        )
target_link_libraries(vv_encode_decode
        gtest gtest_main
        ${shareDT_PLATFORM_LIBS}
        ${VVENC_LIBRARY}
        ${VVDEC_LIBRARY}
        ${PNG_LIBRARIES}
        ${BZIP2_LIBRARIES}
        ${ZLIB_LIBRARY}
        )

set(TESTING_CASES ffmpeg_encode_decode vv_encode_decode
        )

