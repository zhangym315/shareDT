##### ShareDTServer source
set(SHAREDT_SERVER_SRC_COMMON
        ${CAPTURE_SRC}
        ${CMAKE_SOURCE_DIR}/src/capture/Capture.cpp
        ${CMAKE_SOURCE_DIR}/src/capture/ImageRect.cpp
        ${CMAKE_SOURCE_DIR}/src/capture/WindowProcessor.cpp
        ${CMAKE_SOURCE_DIR}/src/capture/SamplesProvider.cpp
        ${CMAKE_SOURCE_DIR}/src/capture/ExportAll.cpp
        ${CMAKE_SOURCE_DIR}/src/capture/CaptureInfo.cpp
        ${CMAKE_SOURCE_DIR}/src/capture/ScreenProvider.cpp
)

set(SHAREDT_SERVER_SRC_COMMON
        ${SHAREDT_SRC}/main/MainService.cpp
        ${SHAREDT_SRC}/main/MainManagementProcess.cpp
        ${SHAREDT_SRC}/capture/WindowsProvider.cpp
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
        ${SHAREDT_SRC}/util/TimeUtil.c
        ${SHAREDT_SRC}/util/Converter.cpp
        ${SHAREDT_SERVER_SRC_COMMON}
)

set(SHAREDT_SERVER_SRC
        ${SHAREDT_SRC}/capture/CaptureServer.cpp
        ${SHAREDT_SERVER_SRC_COMMON}
        ${SHAREDT_SRC}/ffmpeg/ReadWriteVideo.c
        ${SHAREDT_SRC}/ffmpeg/ReadWriteImages.c
        ${SHAREDT_SRC}/cli/ExportImages.cpp
        ${SHAREDT_SRC}/main/MainConsoleSubFunction.cpp
        ${SHAREDT_SRC}/input/InputInterface.cpp
)

set(SHAREDT_SERVER_INCLUDE
        ${CMAKE_BINARY_DIR}/libvnc/
        ${CMAKE_SOURCE_DIR}/src/
        ${CMAKE_SOURCE_DIR}/src/capture/
        ${CMAKE_SOURCE_DIR}/src/util/
        ${CMAKE_SOURCE_DIR}/src/cli/
        ${CMAKE_SOURCE_DIR}/contrib/libwxWidgets/include/
        ${CMAKE_BINARY_DIR}/lib/
        ${SHAREDT_SRC}/ffmpeg/
        ${SHAREDT_SRC}/input/
        ${CMAKE_BINARY_DIR}/lib/wx/include/osx_cocoa-unicode-static-3.1/
)

set(SHAREDT_LIBS_INCLUDE
        ${ZLIB_INCLUDE_DIR}
        ${X265_INCLUDE_DIR}
        ${PNG_INCLUDE_DIR}
        ${FFMPEG_INCLUDE_DIR}
        ${X264_INCLUDE_DIR}
)

##### ShareDTServer Gui source
set(SERVERGUI_SOURCES
        ${SHAREDT_SRC}/sgui/main.cpp
        ${SHAREDT_SRC}/sgui/ServerMainWindow.cpp
        ${SHAREDT_SRC}/sgui/ServerMainWindow.h
        ${SHAREDT_SRC}/sgui/ServerMainWindow.ui
)

set(SHAREDT_CLIENT_GUI_SRC_FILES
        ${SHAREDT_SRC_CLIENT}/gui/ShareDTClientWin.cpp
        ${SHAREDT_SRC_CLIENT}/gui/ShareDTClientWin.ui
)

##### ShareDTClient
set(SHAREDT_CLIENT_SRC_FILES
        ${SHAREDT_SRC}/util/Pid.c
        ${SHAREDT_SRC}/util/Buffer.cpp
        ${SHAREDT_SRC}/util/TimeUtil.c
        ${SHAREDT_SRC_CLIENT}/gui/ShareDTClientWin.cpp
        ${SHAREDT_SRC_CLIENT}/gui/ShareDTClientWin.h
        ${SHAREDT_SRC_CLIENT}/gui/ShareDTClientWin.ui
        ${SHAREDT_SRC_CLIENT}/vnc/FetchingData.cpp
        ${SHAREDT_SRC_CLIENT}/qt/SDThread.cpp
        ${SHAREDT_SRC_CLIENT}/Main.cpp
        ${SHAREDT_SRC}/util/Logger.cpp
)

set(SHAREDT_CLIENT_INCLUDE
        ${SHAREDT_SRC_CLIENT}/gui/
        ${SHAREDT_SRC_CLIENT}/vnc/
        ${SHAREDT_SRC_CLIENT}/qt/
)

set(KeyCode ${CMAKE_SOURCE_DIR}/src/input/KeyCode.txt)