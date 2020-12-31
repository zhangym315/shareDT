
set(X265_INSTALL ${CMAKE_SOURCE_CONTRIB}/x265_3.3/source/build/install/)
set(X265_INCLUDE_DIR ${X265_INSTALL}/include/)

if(WIN32)
    set(X265_LIBRARIES ${X265_INSTALL}/lib/x265-static.lib ${X265_INSTALL}/lib/libx265.lib)
    set(X265_LIBRARY ${X265_LIBRARIES})
else()
    set(X265_LIBRARIES ${X265_INSTALL}/lib/libx265.a)
    set(X265_LIBRARY ${X265_LIBRARIES})
endif()

message(STATUS "Found X265 at ${X265_INSTALL}")
set(X265_FOUND TRUE)
