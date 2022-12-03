# CMakeLists.txt for building the whole project
cmake_minimum_required(VERSION 3.16)
project(shareDT VERSION 0.0.1 LANGUAGES C CXX)

# C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# target executable
set(START_SERVER ShareDT)
set(START_CLIENT ShareDTClient)
set(SERVERGUI_TARGET ServerGui)

# setting
set(CMAKE_SOURCE_CONTRIB ${CMAKE_SOURCE_DIR}/contrib)
set(CMAKE_SOURCE_CMAKE ${CMAKE_SOURCE_DIR}/cmake)
set(CMAKE_SOURCE_CMAKE_CONTRIB ${CMAKE_SOURCE_DIR}/cmake/contrib)
set(SHAREDT_SRC ${CMAKE_SOURCE_DIR}/src)
set(SHAREDT_SRC_CLIENT ${SHAREDT_SRC}/client)
set(CMAKE_SOURCE_LIBVNC ${CMAKE_SOURCE_DIR}/libvnc)

include(${CMAKE_SOURCE_CMAKE}/functions.cmake)
include(${CMAKE_SOURCE_CMAKE}/libs.cmake)
include(${CMAKE_SOURCE_CMAKE}/sources.cmake)

# include headers
include_directories (
    ${LIBVNC_INCLUDE}
    ${SHAREDT_SERVER_INCLUDE}
    ${SHAREDT_LIBS_INCLUDE}
    ${SHAREDT_CLIENT_INCLUDE}
)

### ShareDTServer gui
add_executable(${SERVERGUI_TARGET}
    ${SERVERGUI_SOURCES}
)

target_link_libraries(${SERVERGUI_TARGET} ${SHAREDT_SERVER_QT_LIBS})

set(SERVERGUI_ICONS
    ${SHAREDT_SRC}/sgui/icons/monitor.png
    ${SHAREDT_SRC}/sgui/icons/window.png
    ${SHAREDT_SRC}/sgui/icons/callapse.png
    ${SHAREDT_SRC}/sgui/icons/expand.png
)

#### ShareDTClient  removed client due to ShareDT will repace it
add_executable(${START_CLIENT}
    ${SHAREDT_CLIENT_SRC_FILES}
    ${SHAREDT_CLIENT_GUI_SRC_FILES}
)

target_link_libraries(${START_CLIENT}
    vncclient
    ${PNG_LIBRARIES}
    ${${PROJECT_NAME}_PLATFORM_LIBS}
    ${SHAREDT_CLIENT_QT_LIBS}
    ${FFMPEG_REQUIRED_LIBS}
    ${LZMA_LIBRARIES}
    ${X265_LIBRARIES}
    ${X264_LIBRARY}
)

### Installing
##### ShareDT
set(SHAREDT_MAIN_WINDOW ShareDT)
add_executable(${SHAREDT_MAIN_WINDOW}
        ${SHAREDT_SERVER_SRC}
        ${SHAREDT_SRC}/util/TimeUtil.c
        ${SHAREDT_SRC}/main/Layout.cpp
        ${SHAREDT_SRC}/main/ShareDT.cpp
        ${SHAREDT_SRC}/main/ShareDTWindow.cpp
        ${SHAREDT_SRC}/main/LocalDisplayer.cpp
        ${SHAREDT_SRC_CLIENT}/qt/SDThread.cpp
        ${SHAREDT_SERVER_SRC_COMMON}
        src/cli/RemoteGetter.h src/cli/RemoteGetter.cpp)

target_link_libraries(${SHAREDT_MAIN_WINDOW}
        ${SHAREDT_CLIENT_QT_LIBS}
        ${${PROJECT_NAME}_PLATFORM_LIBS}

        ${BZIP2_LIBRARIES}
        ${FFMPEG_REQUIRED_LIBS}
        vncserver
        ${X265_LIBRARIES}
        ${X264_LIBRARY}
        ${LZMA_LIBRARIES}
        )

if(APPLE)
        install(TARGETS ${SHAREDT_MAIN_WINDOW} DESTINATION ${SHAREDT_MAIN_WINDOW}.app/Contents/MacOS)
        install(FILES ${CMAKE_SOURCE_DIR}/image/ShareDT.icns DESTINATION ${SHAREDT_MAIN_WINDOW}.app/Contents/Resources)
        install(FILES ${CMAKE_SOURCE_DIR}/install/Info.plist DESTINATION ${SHAREDT_MAIN_WINDOW}.app/Contents)

        install(FILES ${KeyCode} DESTINATION ${SHAREDT_MAIN_WINDOW}.app/Contents/MacOS )
else()
        install(TARGETS ${SHAREDT_MAIN_WINDOW} DESTINATION ShareDT/bin)
        install(FILES ${CMAKE_SOURCE_DIR}/image/ShareDT.png DESTINATION ShareDT/bin)
        install(DIRECTORY DESTINATION ShareDT/var/run/)

        install(FILES ${KeyCode} DESTINATION ShareDT/bin )
endif()

if(WIN32)
        install(TARGETS ${SHAREDT_MAIN_WINDOW} DESTINATION MSI/)
        install(FILES ${CMAKE_SOURCE_DIR}/image/ShareDT.ico DESTINATION MSI/)
        install(FILES ${CMAKE_SOURCE_DIR}/image/ShareDT.png DESTINATION MSI/)
        install(FILES ${CMAKE_SOURCE_DIR}/image/ShareDTDiaglogInstall.png DESTINATION MSI/)
        install(FILES ${CMAKE_SOURCE_DIR}/image/ShareDTDiagBannerInstall.png DESTINATION MSI/)
        install(FILES ${CMAKE_SOURCE_DIR}/install/ShareDT.wxs DESTINATION MSI/)
endif()

#### Testing #### Can't link gtest on Win32, skip win testing
if(NOT WIN32)
add_subdirectory( testing )
endif()
