# CMakeLists.txt for building the whole project
cmake_minimum_required(VERSION 3.4)
project(shareDT VERSION 0.0.1 LANGUAGES C CXX)

# C++17
set(CMAKE_CXX_STANDARD 17)

# target executable
set(START_SERVER ShareDTServer)
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

### ShareDTServer
add_executable(${START_SERVER} ${SHAREDT_SERVER_SRC})

target_link_libraries(${START_SERVER}
    ${LZMA_LIBRARIES}
    ${BZIP2_LIBRARIES}
    ${FFMPEG_REQUIRED_LIBS}
    vncserver
    ${${PROJECT_NAME}_PLATFORM_LIBS}
    ${X265_LIBRARIES}
    ${X264_LIBRARY}
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

#### ShareDTClient
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
# ShareDTServer
install(TARGETS ${START_SERVER} DESTINATION server/bin)
install(FILES ${KeyCode} DESTINATION server/bin )
install(FILES ${SERVERGUI_ICONS} DESTINATION server/images)
install(DIRECTORY DESTINATION server/var/run/)

# ShareDTClient
install(TARGETS ${START_CLIENT} DESTINATION client/bin)



##### Local Displayer
set(SHAREDT_LOCAL_DISPLAYER LocalDisplayer)
add_executable(${SHAREDT_LOCAL_DISPLAYER}
                ${SHAREDT_SRC}/local/LocalDisplayer.cpp
                ${SHAREDT_SRC}/local/LocalDisplayer.ui
                ${SHAREDT_SRC_CLIENT}/qt/SDThread.cpp
                ${SHAREDT_SERVER_SRC_COMMON}
                )

target_link_libraries(${SHAREDT_LOCAL_DISPLAYER}
                      ${SHAREDT_CLIENT_QT_LIBS}
                      ${${PROJECT_NAME}_PLATFORM_LIBS}
         )


if(APPLE)
    install(TARGETS ${SHAREDT_LOCAL_DISPLAYER} DESTINATION LocalDisplayer.app/Contents/MacOS)
endif()

##### ShareDTWindow
set(SHAREDT_MAIN_WINDOW ShareDT)
add_executable(${SHAREDT_MAIN_WINDOW}
        ${SHAREDT_SRC}/main/MainWindow.cpp
        ${SHAREDT_SRC}/main/Layout.cpp
        ${SHAREDT_SRC}/main/ShareDTWindow.cpp
        ${SHAREDT_SERVER_SRC_COMMON}
        )

target_link_libraries(${SHAREDT_MAIN_WINDOW}
        ${SHAREDT_CLIENT_QT_LIBS}
        ${${PROJECT_NAME}_PLATFORM_LIBS}
        )


#### Testing #### Can't link gtest on Win32, skip win testing
if(NOT WIN32)
add_subdirectory( testing )
endif()