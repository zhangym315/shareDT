
# lib
set(WITH_ZLIB ON)
add_subdirectory(${CMAKE_SOURCE_DIR}/libvnc)

# include Lib
set(LIBVNC_INCLUDE ${CMAKE_SOURCE_DIR}/libvnc ${CMAKE_SOURCE_DIR}/libvnc/libvncserver ${CMAKE_SOURCE_DIR}/libvnc/common)

# add exec file name
add_definitions(-DSTART_SERVER_EXEC="${START_SERVER}")

# zlib
set(ZLIB_DIR ${CMAKE_SOURCE_CMAKE_CONTRIB})
find_package(ZLIB REQUIRED NO_MODULE)

# x265
set(X265_DIR ${CMAKE_SOURCE_CMAKE_CONTRIB})
find_package(X265 REQUIRED NO_MODULE)

# png
set(png_DIR ${CMAKE_SOURCE_CMAKE_CONTRIB})
find_package(png REQUIRED NO_MODULE)

set(ffmpeg_DIR ${CMAKE_SOURCE_CMAKE_CONTRIB})
find_package(ffmpeg REQUIRED NO_MODULE)

set(bzip2_DIR ${CMAKE_SOURCE_CMAKE_CONTRIB})
find_package(bzip2 REQUIRED NO_MODULE)

set(lzma_DIR ${CMAKE_SOURCE_CMAKE_CONTRIB})
find_package(lzma REQUIRED NO_MODULE)

set(x264_DIR ${CMAKE_SOURCE_CMAKE_CONTRIB})
find_package(x264 REQUIRED NO_MODULE)

#### QT specific
# QMake specific
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
# Instruct CMake to run uic automatically when needed.
set(CMAKE_AUTOUIC ON)

# This will find the Qt6 files. You will need a Qt6_DIR env variable
set(QT660BUILD_LIB_CMAKE "${CMAKE_SOURCE_CONTRIB}/qt660/static-build/lib/cmake/")
include_directories("${CMAKE_SOURCE_CONTRIB}/qt660/static-build/include")

set(Qt6Widgets_DIR              "${QT660BUILD_LIB_CMAKE}/Qt6Widgets")
set(Qt6WidgetsTools_DIR         "${QT660BUILD_LIB_CMAKE}/Qt6WidgetsTools")
set(Qt6CoreTools_DIR            "${QT660BUILD_LIB_CMAKE}/Qt6CoreTools")
set(Qt6GuiTools_DIR             "${QT660BUILD_LIB_CMAKE}/Qt6GuiTools")
set(Qt6BundledZLIB_DIR          "${QT660BUILD_LIB_CMAKE}/Qt6BundledZLIB")
set(Qt6BundledPcre2_DIR         "${QT660BUILD_LIB_CMAKE}/Qt6BundledPcre2")
set(Qt6BundledLibpng_DIR        "${QT660BUILD_LIB_CMAKE}/Qt6BundledLibpng")
set(Qt6BundledHarfbuzz_DIR      "${QT660BUILD_LIB_CMAKE}/Qt6BundledHarfbuzz")
set(Qt6BundledFreetype_DIR      "${QT660BUILD_LIB_CMAKE}/Qt6BundledFreetype")
set(Qt6BundledLibjpeg_DIR       "${QT660BUILD_LIB_CMAKE}/Qt6BundledLibjpeg")
set(Qt6DBusTools_DIR            "${QT660BUILD_LIB_CMAKE}/Qt6DBusTools")

set(Qt6_DIR "${QT660BUILD_LIB_CMAKE}/Qt6")
message("qt6: ${QT660BUILD_LIB_CMAKE}")
find_package(Qt6 COMPONENTS Core Widgets REQUIRED)


set(SHAREDT_SERVER_QT_LIBS Qt6::Widgets)
set(SHAREDT_CLIENT_QT_LIBS Qt6::Core Qt6::Gui Qt6::Widgets)

################ BEGIN Platform specific source and libs
if(WIN32)
    set(${PROJECT_NAME}_PLATFORM_LIBS Dwmapi DXGI
        Secur32 Strmiids Mfuuid Bcrypt ### ffmpeg
        crypt32   ### openssl
        )
    add_definitions(-DNOMINMAX)
    add_definitions(-D__SHAREDT_WIN__)

    set(CAPTURE_SRC
            ${CMAKE_SOURCE_DIR}/src/capture/win/WinGetWindows.cpp
            ${CMAKE_SOURCE_DIR}/src/capture/win/WinGetMonitors.cpp
            ${CMAKE_SOURCE_DIR}/src/capture/win/GDIWindowProcessor.cpp
            ${CMAKE_SOURCE_DIR}/src/input/win/MouseEvents.cpp
            ${CMAKE_SOURCE_DIR}/src/input/win/KeyboardEvents.cpp
            ${CMAKE_SOURCE_DIR}/src/service/MainWindowsService.cpp
            ${CMAKE_SOURCE_DIR}/src/util/WindowsProcess.cpp
            )

    # libs for FFMPEG
    set(FFMPEG_REQUIRED_LIBS "")

elseif(APPLE)
    add_definitions(-D__SHAREDT_IOS__)
    find_package(Threads REQUIRED)
    find_library(corefoundation_lib CoreFoundation REQUIRED)
    find_library(cocoa_lib Cocoa REQUIRED)
    find_library(coremedia_lib CoreMedia REQUIRED)
    find_library(avfoundation_lib AVFoundation REQUIRED)
    find_library(coregraphics_lib CoreGraphics REQUIRED)
    find_library(corevideo_lib CoreVideo REQUIRED)

    # For ffmpeg library linkage
    find_library(AudioToolbox_LIB AudioToolbox REQUIRED)
    find_library(CoreFoundation_LIB CoreFoundation REQUIRED)
    find_library(CoreMedia_LIB CoreMedia REQUIRED)
    find_library(CoreVideo_LIB CoreVideo REQUIRED)
    find_library(VideoToolbox_LIB VideoToolbox REQUIRED)
    find_library(VideoToolbox_Security Security REQUIRED)

    set(FFMPEG_REQUIRED_LIBS
            ${FFMPEG_LIBRARIES}
            ${AudioToolbox_LIB}
            ${CoreFoundation_LIB}
            ${CoreMedia_LIB}
            ${CoreVideo_LIB}
            ${VideoToolbox_LIB}
            ${VideoToolbox_Security}
            )

    set(CAPTURE_SRC
            ${CMAKE_SOURCE_DIR}/src/capture/ios/CGGetWindows.cpp
            ${CMAKE_SOURCE_DIR}/src/capture/ios/CGGetMonitors.cpp
            ${CMAKE_SOURCE_DIR}/src/capture/ios/NSFrame.mm
            ${CMAKE_SOURCE_DIR}/src/capture/ios/CGWindowsFrame.cpp
            ${CMAKE_SOURCE_DIR}/src/input/ios/KeyboardEvents.mm
            ${CMAKE_SOURCE_DIR}/src/input/ios/MouseEvents.mm
            ${CMAKE_SOURCE_DIR}/src/service/MainNixService.cpp
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

elseif(UNIX)
    add_definitions(-D__SHAREDT_LINUX__)
    find_package(X11 REQUIRED)
    if(!X11_XTest_FOUND)
        message(FATAL_ERROR "X11 extensions are required, but not f.
        ound!")
    endif()
    if(!X11_Xfixes_LIB)
        message(FATAL_ERROR "X11 fixes extension is required, but not found!")
    endif()
    find_package(Threads REQUIRED)
    set(CAPTURE_SRC
            ${CMAKE_SOURCE_DIR}/src/capture/linux/XGetWindows.cpp
            ${CMAKE_SOURCE_DIR}/src/capture/linux/XGetMonitors.cpp
            ${CMAKE_SOURCE_DIR}/src/capture/linux/XFrameProcessor.cpp
            ${CMAKE_SOURCE_DIR}/src/input/linux/MouseEvents.cpp
            ${CMAKE_SOURCE_DIR}/src/input/linux/KeyboardEvents.cpp
            ${CMAKE_SOURCE_DIR}/src/service/MainNixService.cpp
            )

    set(${PROJECT_NAME}_PLATFORM_LIBS
            ${X11_LIBRARIES}
            ${X11_Xfixes_LIB}
            ${X11_XTest_LIB}
            ${X11_Xinerama_LIB}
            ${CMAKE_THREAD_LIBS_INIT}
            -ldl
            )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DwxUSE_THREADS=1 -DLINK_LEXERS -DNO_CXX11_REGEX -DSCI_LEXER -DWXBUILDING -DWX_PRECOMP -D_LIB -D_UNICODE -D__WXGTK2__ -D__WXGTK__ -D__WX__ -DwxUSE_BASE=1 -DwxUSE_GUI=1")

    # libs for FFMPEG
    set(FFMPEG_REQUIRED_LIBS
            ${FFMPEG_LIBRARIES}
            )
else()
    message(FATAL_ERROR " Not supported platform ${ARGS}")
endif()
################ END   Platform specific libs
