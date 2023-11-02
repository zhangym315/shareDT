
set(VVENC_INSTALL ${CMAKE_SOURCE_CONTRIB}/vvenc/build/install/)
set(VVENC_INCLUDE_DIR ${VVENC_INSTALL}/include/)
if(WIN32)
    set(VVENC_LIBRARIES ${VVENC_INSTALL}/lib/vvenc.lib)
    set(VVENC_LIBRARY ${VVENC_LIBRARIES})
else()
    set(VVENC_LIBRARIES ${VVENC_INSTALL}/lib/libvvenc.a)
    set(VVENC_LIBRARY ${VVENC_LIBRARIES})
endif()

message(STATUS "Found VVENC at ${VVENC_INSTALL}")
set(VVENC_FOUND TRUE)
