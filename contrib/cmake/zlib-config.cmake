
set(ZLIB_INSTALL ${CMAKE_SOURCE_CONTRIB}/zlib/build/install/)
set(ZLIB_INCLUDE_DIR ${ZLIB_INSTALL}/include/)
set(ZLIB_LIBRARIES ${ZLIB_INSTALL}/lib/libz.a)

message(STATUS "Found ZLIB at ${ZLIB_INSTALL}")
set(ZLIB_FOUND TRUE)