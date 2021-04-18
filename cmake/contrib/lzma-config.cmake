set(LZMA_INSTALL ${CMAKE_SOURCE_CONTRIB}/liblzma/build/install/)
set(LZMA_INCLUDE_DIR ${LZMA_INSTALL}/include/)
set(LZMA_LIBRARIES
        ${LZMA_INSTALL}/lib/liblzma.a
        )

foreach(x IN ITEMS ${LZMA_LIBRARIES})
    check_existing(${x})
endforeach()
check_existing(${LZMA_INCLUDE_DIR})

message(STATUS "Found LZMA at ${LZMA_INSTALL}")
set(LZMA_FOUND TRUE)