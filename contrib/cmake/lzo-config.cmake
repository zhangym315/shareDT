
set(LZO_INSTALL ${CMAKE_SOURCE_CONTRIB}/lzo-2.10/build/install/)
set(LZO_INCLUDE_DIR ${LZO_INSTALL}/include/)
if(WIN32)
    set(LZO_LIBRARY ${LZO_INSTALL}/lib/lzo2.lib)
    set(LZO_LIBRARIES ${LZO_INSTALL}/lib/lzo2.lib)
else()
    set(LZO_LIBRARY ${LZO_INSTALL}/lib/liblzo2.a)
    set(LZO_LIBRARIES ${LZO_INSTALL}/lib/liblzo2.a)
endif()

message(STATUS "Found LZO at ${LZO_INSTALL}")
set(LZO_FOUND TRUE)
