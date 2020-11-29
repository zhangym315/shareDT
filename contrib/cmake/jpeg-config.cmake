
set(JPEG_INSTALL ${CMAKE_SOURCE_CONTRIB}/libjpeg-turbo-2.0.5/build/install/)
set(JPEG_INCLUDE_DIR ${JPEG_INSTALL}/include/)
set(JPEG_LIBRARY ${JPEG_INSTALL}/lib/libjpeg.a ${JPEG_INSTALL}/lib/libturbojpeg.a )
set(JPEG_LIBRARIES ${JPEG_INSTALL}/lib/libjpeg.a ${JPEG_INSTALL}/lib/libturbojpeg.a )

message(STATUS "Found JPEG at ${JPEG_INSTALL}")
set(JPEG_FOUND TRUE)
