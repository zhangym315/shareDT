
set(ZLIB_INSTALL ${CMAKE_SOURCE_CONTRIB}/zlib/build/install/)
set(ZLIB_INCLUDE_DIR ${ZLIB_INSTALL}/include/)
if(WIN32)
    set(ZLIB_LIBRARIES ${ZLIB_INSTALL}/lib/zlibstaticd.lib)
    set(ZLIB_LIBRARY ${ZLIB_LIBRARIES})
else()
    set(ZLIB_LIBRARIES ${ZLIB_INSTALL}/lib/libz.a)
    set(ZLIB_LIBRARY ${ZLIB_LIBRARIES})
endif()

message(STATUS "Found ZLIB at ${ZLIB_INSTALL}")
set(ZLIB_FOUND TRUE)
