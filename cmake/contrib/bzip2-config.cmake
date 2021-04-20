set(BZIP2_INSTALL ${CMAKE_SOURCE_CONTRIB}/bzip2/build/install/)
set(BZIP2_INCLUDE_DIR ${BZIP2_INSTALL}/include/)
set(BZIP2_LIBRARIES
        ${BZIP2_INSTALL}/lib/libbz2.a
        )

foreach(x IN ITEMS ${BZIP2_LIBRARIES})
    check_existing(${x})
endforeach()
check_existing(${BZIP2_INCLUDE_DIR})

message(STATUS "Found BZIP2 at ${BZIP2_INSTALL}")
set(BZIP2_FOUND TRUE)