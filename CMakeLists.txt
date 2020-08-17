# CMakeLists.txt for building the whole project
cmake_minimum_required(VERSION 3.4)
project(shareDT VERSION 0.0.1 LANGUAGES C CXX)

# C++14
set(CMAKE_CXX_STANDARD 17)

# set START_SERVER
set(START_SERVER ShareDTServer)

# QMake specific
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
# Instruct CMake to run uic automatically when needed.
set(CMAKE_AUTOUIC ON)
# This will find the Qt5 files. You will need a QT5_DIR env variable
find_package(Qt5Widgets REQUIRED) # Equivalent of QT += widgets

set(SERVERGUI_SOURCES
        src/sgui/main.cpp
        src/sgui/ServerMainWindow.cpp)
set(SERVERGUI_HEADERS
        src/sgui/ServerMainWindow.h)
set(SERVERGUI_UI
        src/sgui/ServerMainWindow.ui)
set(SERVERGUI_TARGET ServerGui)

# This will create you executable
add_executable(${SERVERGUI_TARGET} ${SERVERGUI_SOURCES} ${SERVERGUI_HEADERS} ${SERVERGUI_UI})
# This will link necessary Qt5 libraries to your project
target_link_libraries(${SERVERGUI_TARGET} Qt5::Widgets)

# C/C++ flags
#set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -Wdeprecated-declarations ")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused-variable")

# lib 
add_subdirectory(${CMAKE_SOURCE_DIR}/contrib/libvnc)

# include Lib
set(LIBVNC_INCLUDE ${CMAKE_SOURCE_DIR}/contrib/libvnc ${CMAKE_SOURCE_DIR}/contrib/libvnc/libvncserver ${CMAKE_SOURCE_DIR}/contrib/libvnc/common)

# add exec file name
add_definitions(-DSTART_SERVER_EXEC="${START_SERVER}")

# include headers
# ${CMAKE_CURRENT_BINARY_DIR}/libvnc/ for the generated rfbconfig.h
include_directories (
    /usr/local/include/
    ${CMAKE_BINARY_DIR}/contrib/libvnc/
    ${CMAKE_SOURCE_DIR}/contrib/libvnc/
    ${CMAKE_SOURCE_DIR}/contrib/libvnc/libvncserver/
    ${CMAKE_SOURCE_DIR}/contrib/libvnc/common/
    ${CMAKE_SOURCE_DIR}/src/
    ${CMAKE_SOURCE_DIR}/src/capture/
    ${CMAKE_SOURCE_DIR}/src/util/
    ${CMAKE_SOURCE_DIR}/contrib/libwxWidgets/include/
    ${CMAKE_BINARY_DIR}/lib/
)

set(SHAREDT_SRC ${CMAKE_SOURCE_DIR}/src)
set(CAPTURE_SRC
    ${CMAKE_SOURCE_DIR}/src/capture/ImageRect.cpp
    ${CMAKE_SOURCE_DIR}/src/capture/WindowProcessor.cpp
    ${CMAKE_SOURCE_DIR}/src/capture/SamplesProvider.cpp
    )

# platform dependent settings
if(WIN32)
    set(${PROJECT_NAME}_PLATFORM_LIBS Dwmapi DXGI)
    add_definitions(-DNOMINMAX)
    add_definitions(-D__SHAREDT_WIN__)

    set(CAPTURE_SRC
        ${CAPTURE_SRC}
        ${CMAKE_SOURCE_DIR}/src/capture/win/WinGetWindows.cpp
        ${CMAKE_SOURCE_DIR}/src/capture/win/WinGetMonitors.cpp
        ${CMAKE_SOURCE_DIR}/src/capture/win/GDIWindowProcessor.cpp
        ${CMAKE_SOURCE_DIR}/src/capture/win/GDIWindowProcessor.cpp
        ${CMAKE_SOURCE_DIR}/src/MainWindowsService.cpp
        )
elseif(APPLE)
    add_definitions(-D__SHAREDT_IOS__)
    find_package(Threads REQUIRED)
    find_library(corefoundation_lib CoreFoundation REQUIRED)
    find_library(cocoa_lib Cocoa REQUIRED)
    find_library(coremedia_lib CoreMedia REQUIRED)
    find_library(avfoundation_lib AVFoundation REQUIRED)
    find_library(coregraphics_lib CoreGraphics REQUIRED)
    find_library(corevideo_lib CoreVideo REQUIRED)

    set(CAPTURE_SRC
        ${CAPTURE_SRC}
        ${CMAKE_SOURCE_DIR}/src/capture/ios/CGWindowProcessor.cpp
        ${CMAKE_SOURCE_DIR}/src/capture/ios/CGGetWindows.cpp
        ${CMAKE_SOURCE_DIR}/src/capture/ios/CGGetMonitors.cpp
        ${CMAKE_SOURCE_DIR}/src/capture/ios/NSFrame.mm
        ${CMAKE_SOURCE_DIR}/src/capture/ios/CGWindowsFrame.cpp
        ${CMAKE_SOURCE_DIR}/src/MainNixService.cpp
        ${SHAREDT_SRC}/util/Sock.cpp
    )
    set(${PROJECT_NAME}_PLATFORM_LIBS
        ${CMAKE_THREAD_LIBS_INIT}
        ${corefoundation_lib}
        ${cocoa_lib}
        ${coremedia_lib}
        ${avfoundation_lib}
        ${coregraphics_lib}
        ${corevideo_lib}
    )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DwxUSE_THREADS=1 -DWXUSINGDLL -D__WXOSX_COCOA__ -D__WXMAC__ -D__WXOSX__ -pthread ")
else()
    add_definitions(-D__SHAREDT_LINUX__)
    find_package(X11 REQUIRED)
    if(!X11_XTest_FOUND)
        message(FATAL_ERROR "X11 extensions are required, but not found!")
    endif()
    if(!X11_Xfixes_LIB)
        message(FATAL_ERROR "X11 fixes extension is required, but not found!")
    endif()
    find_package(Threads REQUIRED)
    set(CAPTURE_SRC
        ${CAPTURE_SRC}
        ${CMAKE_SOURCE_DIR}/src/capture/linux/XGetWindows.cpp
        ${CMAKE_SOURCE_DIR}/src/capture/linux/XGetMonitors.cpp
        ${CMAKE_SOURCE_DIR}/src/capture/linux/XFrameProcessor.cpp
        ${CMAKE_SOURCE_DIR}/src/MainNixService.cpp
        ${SHAREDT_SRC}/util/Sock.cpp
    )

    set(${PROJECT_NAME}_PLATFORM_LIBS
        ${X11_LIBRARIES}
        ${X11_Xfixes_LIB}
        ${X11_XTest_LIB}
        ${X11_Xinerama_LIB}
        ${CMAKE_THREAD_LIBS_INIT}
    )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DwxUSE_THREADS=1 -DLINK_LEXERS -DNO_CXX11_REGEX -DSCI_LEXER -DWXBUILDING -DWX_PRECOMP -D_LIB -D_UNICODE -D__WXGTK2__ -D__WXGTK__ -D__WX__ -DwxUSE_BASE=1 -DwxUSE_GUI=1")
endif()

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
        ${SHAREDT_SRC}/util/Pid.c
        ${CAPTURE_SRC}
        )
#target_link_libraries(${START_SERVER} vncserver screen_capture_lite ${${PROJECT_NAME}_PLATFORM_LIBS} )
target_link_libraries(${START_SERVER} vncserver  ${${PROJECT_NAME}_PLATFORM_LIBS} )

#set(VNCSCREENCAP vncserver screen_capture_lite ${${PROJECT_NAME}_PLATFORM_LIBS})
set(VNCSCREENCAP vncserver  ${${PROJECT_NAME}_PLATFORM_LIBS})

# set icons
set(SERVERGUI_ICONS
        ${SHAREDT_SRC}/sgui/icons/monitor.png
        ${SHAREDT_SRC}/sgui/icons/window.png
        ${SHAREDT_SRC}/sgui/icons/callapse.png
        ${SHAREDT_SRC}/sgui/icons/expand.png
        )

# Installing
install(TARGETS ${SERVERGUI_TARGET} ${START_SERVER} DESTINATION bin)
install(FILES ${SERVERGUI_ICONS} DESTINATION images)
install(DIRECTORY DESTINATION var/run/)