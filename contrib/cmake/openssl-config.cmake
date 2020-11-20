
set(OPENSSL_INSTALL ${CMAKE_SOURCE_CONTRIB}/openssl/build/install/)
set(OPENSSL_INCLUDE_DIR ${OPENSSL_INSTALL}/include)
set(OPENSSL_LIBRARIES ${OPENSSL_INSTALL}/lib/libssl.a ${OPENSSL_INSTALL}/lib/libcrypto.a)

message(STATUS "Found OpenSSL at ${OPENSSL_INSTALL}")
set(OPENSSL_FOUND TRUE)
