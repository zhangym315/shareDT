
set(LZO_INSTALL ${CMAKE_SOURCE_CONTRIB}/lzo-2.10/build/install/)
set(LZO_INCLUDE_DIR ${LZO_INSTALL}/include/)
set(LZO_LIBRARY ${LZO_INSTALL}/lib/liblzo2.a)
set(LZO_LIBRARIES ${LZO_INSTALL}/lib/liblzo2.a)

message(STATUS "Found LZO at ${LZO_INSTALL}")
set(LZO_FOUND TRUE)
