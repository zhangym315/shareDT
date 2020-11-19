
set(PNG_INSTALL ${CMAKE_SOURCE_CONTRIB}/libpng/build/install/)
set(PNG_INCLUDE_DIR ${PNG_INSTALL}/include/)
set(PNG_LIBRARIES ${PNG_INSTALL}/lib/libpng.a)

message(STATUS "Found PNG at ${PNG_INSTALL}")
set(PNG_FOUND TRUE)
