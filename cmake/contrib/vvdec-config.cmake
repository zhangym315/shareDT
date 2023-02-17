
set(VVDEC_INSTALL ${CMAKE_SOURCE_CONTRIB}/vvdec/build/install/)
set(VVDEC_INCLUDE_DIR ${VVDEC_INSTALL}/include/)
if(WIN32)
    set(VVDEC_LIBRARIES ${VVDEC_INSTALL}/lib/vvdec.lib)
    set(VVDEC_LIBRARY ${VVDEC_LIBRARIES})
else()
    set(VVDEC_LIBRARIES ${VVDEC_INSTALL}/lib/libvvdec.a)
    set(VVDEC_LIBRARY ${VVDEC_LIBRARIES})
endif()

message(STATUS "Found VVDEC at ${VVDEC_INSTALL}")
set(VVDEC_FOUND TRUE)
