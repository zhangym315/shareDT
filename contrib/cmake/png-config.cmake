
set(PNG_INSTALL ${CMAKE_SOURCE_CONTRIB}/libpng-1.6.37/build/install/)
set(PNG_INCLUDE_DIR ${PNG_INSTALL}/include/)
if(WIN32)
    set(PNG_LIBRARIES ${PNG_INSTALL}/lib/libpng16_static.lib)
else()
    set(PNG_LIBRARIES ${PNG_INSTALL}/lib/libpng.a)
endif()

message(STATUS "Found PNG at ${PNG_INSTALL}")
set(PNG_FOUND TRUE)
