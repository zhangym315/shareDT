
set(JPEG_INSTALL ${CMAKE_SOURCE_CONTRIB}/libjpeg-turbo/build/install/)
set(JPEG_INCLUDE_DIR ${JPEG_INSTALL}/include/)
if(WIN32)
    set(JPEG_LIBRARY ${JPEG_INSTALL}/lib/jpeg-static.lib ${JPEG_INSTALL}/lib/turbojpeg-static.lib )
    set(JPEG_LIBRARIES ${JPEG_INSTALL}/lib/jpeg-static.lib ${JPEG_INSTALL}/lib/turbojpeg-static.lib )
else()
    set(JPEG_LIBRARY ${JPEG_INSTALL}/lib/libjpeg.a ${JPEG_INSTALL}/lib/libturbojpeg.a )
    set(JPEG_LIBRARIES ${JPEG_INSTALL}/lib/libjpeg.a ${JPEG_INSTALL}/lib/libturbojpeg.a )
endif()

message(STATUS "Found JPEG at ${JPEG_INSTALL}")
set(JPEG_FOUND TRUE)
