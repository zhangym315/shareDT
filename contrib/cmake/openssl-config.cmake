
set(OPENSSL_INSTALL ${CMAKE_SOURCE_CONTRIB}/openssl/build/install/)
set(OPENSSL_INCLUDE_DIR ${OPENSSL_INSTALL}/include)

if(UNIX)
    set(OPENSSL_WITH_DL "-ldl")
endif()

set(OPENSSL_LIBRARIES ${OPENSSL_INSTALL}/lib/libssl.a ${OPENSSL_INSTALL}/lib/libcrypto.a ${OPENSSL_WITH_DL})

message(STATUS "Found OpenSSL at ${OPENSSL_INSTALL}")
set(OPENSSL_FOUND TRUE)
