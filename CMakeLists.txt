# CMakeLists.txt for building the whole project
cmake_minimum_required(VERSION 3.4)
project(shareDT VERSION 0.0.1 LANGUAGES C CXX)

# C++14
set(CMAKE_CXX_STANDARD 17)

# set START_SERVER
set(START_SERVER ShareDTServer)
set(CMAKE_SOURCE_CONTRIB ${CMAKE_SOURCE_DIR}/contrib)
set(CMAKE_SOURCE_CMAKE ${CMAKE_SOURCE_DIR}/cmake)
set(CMAKE_SOURCE_CMAKE_CONTRIB ${CMAKE_SOURCE_DIR}/cmake/contrib)

include(${CMAKE_SOURCE_CMAKE}/functions.cmake)
include(${CMAKE_SOURCE_CMAKE}/libs.cmake)

# QMake specific
# Instruct CMake to run moc automatically when needed.
#set(CMAKE_AUTOMOC ON)
# Instruct CMake to run uic automatically when needed.
#set(CMAKE_AUTOUIC ON)
# This will find the Qt5 files. You will need a QT5_DIR env variable
#set(Qt5Widgets_DIR "${CMAKE_SOURCE_DIR}/contrib/qt515/static-build/lib/cmake/Qt5Widgets")
#find_package(Qt5Widgets REQUIRED NO_MODULE)

set(SERVERGUI_SOURCES
        src/sgui/main.cpp
        src/sgui/ServerMainWindow.cpp)
set(SERVERGUI_HEADERS
        src/sgui/ServerMainWindow.h)
set(SERVERGUI_UI
        src/sgui/ServerMainWindow.ui)
set(SERVERGUI_TARGET ServerGui)

# This will create you executable
#add_executable(${SERVERGUI_TARGET} ${SERVERGUI_SOURCES} ${SERVERGUI_HEADERS} ${SERVERGUI_UI})
# This will link necessary Qt5 libraries to your project
#target_link_libraries(${SERVERGUI_TARGET} Qt5::Widgets)

# C/C++ flags
#set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -Wdeprecated-declarations ")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused-variable")

# include headers
# ${CMAKE_CURRENT_BINARY_DIR}/libvnc/ for the generated rfbconfig.h
include_directories (
    /usr/local/include/
    ${LIBVNC_INCLUDE}
    ${CMAKE_BINARY_DIR}/libvnc/
    ${CMAKE_SOURCE_DIR}/src/
    ${CMAKE_SOURCE_DIR}/src/capture/
    ${CMAKE_SOURCE_DIR}/src/util/
    ${CMAKE_SOURCE_DIR}/src/cli/
    ${CMAKE_SOURCE_DIR}/contrib/libwxWidgets/include/
    ${CMAKE_BINARY_DIR}/lib/
    ${ZLIB_INCLUDE_DIR}
    ${X265_INCLUDE_DIR}
    ${PNG_INCLUDE_DIR}
    ${FFMPEG_INCLUDE_DIR}
)

set(SHAREDT_SRC ${CMAKE_SOURCE_DIR}/src)
set(CAPTURE_SRC
    ${CAPTURE_SRC}
    ${CMAKE_SOURCE_DIR}/src/capture/ImageRect.cpp
    ${CMAKE_SOURCE_DIR}/src/capture/WindowProcessor.cpp
    ${CMAKE_SOURCE_DIR}/src/capture/SamplesProvider.cpp
    )

include_directories(
    ${CMAKE_BINARY_DIR}/lib/wx/include/osx_cocoa-unicode-static-3.1/
)

add_executable(${START_SERVER}
        ${SHAREDT_SRC}/StartServer.cpp
        ${SHAREDT_SRC}/ScreenProvider.cpp
        ${SHAREDT_SRC}/MainService.cpp
        ${SHAREDT_SRC}/MainConsole.cpp
        ${SHAREDT_SRC}/WindowsProvider.cpp
        ${SHAREDT_SRC}/MainManagementProcess.cpp
        ${SHAREDT_SRC}/util/StringTools.cpp
        ${SHAREDT_SRC}/util/Buffer.cpp
        ${SHAREDT_SRC}/util/Thread.cpp
        ${SHAREDT_SRC}/util/Daemon.cpp
        ${SHAREDT_SRC}/util/Logger.cpp
        ${SHAREDT_SRC}/util/Path.cpp
        ${SHAREDT_SRC}/util/Enum.cpp
        ${SHAREDT_SRC}/util/ReadWriteFD.cpp
        ${SHAREDT_SRC}/util/Sock.cpp
        ${SHAREDT_SRC}/util/Pid.c
        ${SHAREDT_SRC}/cli/ExportImages.cpp
        ${SHAREDT_SRC}/ffmpeg/ReadWriteImages.c
        ${SHAREDT_SRC}/ffmpeg/ReadWriteVideo.c
        ${CAPTURE_SRC}
        )

message("capture src: ${CAPTURE_SRC}")
#target_link_libraries(${START_SERVER} vncserver screen_capture_lite ${${PROJECT_NAME}_PLATFORM_LIBS} )
target_link_libraries(${START_SERVER}
        ${BZIP2_LIBRARIES}
        ${LZMA_LIBRARIES}
        ${FFMPEG_LIBRARIES}
        ${FFMPEG_REQUIRED_LIBS}
        vncserver
        ${${PROJECT_NAME}_PLATFORM_LIBS}
        ${X265_LIBRARIES}
        )

#set(VNCSCREENCAP vncserver screen_capture_lite ${${PROJECT_NAME}_PLATFORM_LIBS})
set( VNCSCREENCAP vncserver  ${${PROJECT_NAME}_PLATFORM_LIBS} )

# set icons
set(SERVERGUI_ICONS
        ${SHAREDT_SRC}/sgui/icons/monitor.png
        ${SHAREDT_SRC}/sgui/icons/window.png
        ${SHAREDT_SRC}/sgui/icons/callapse.png
        ${SHAREDT_SRC}/sgui/icons/expand.png
    )

# Installing
#install(TARGETS ${SERVERGUI_TARGET} ${START_SERVER} DESTINATION bin)
install(TARGETS ${START_SERVER} DESTINATION bin)
install(FILES ${SERVERGUI_ICONS} DESTINATION images)
install(DIRECTORY DESTINATION var/run/)
