cmake_minimum_required(VERSION 3.5)

project(Infinity VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories("${PROJECT_SOURCE_DIR}/include")
link_directories("${PROJECT_SOURCE_DIR}/lib")

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Core Gui Multimedia MultimediaWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Core Gui Sql Multimedia MultimediaWidgets)

set(PROJECT_SOURCES
    main.cpp
    infinityplayer.cpp
    infinityplayer.h
    playercontrols.h playercontrols.cpp
    sonic.h sonic.cpp
    player.h player.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Infinity
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Infinity APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Infinity SHARED
            ${PROJECT_SOURCES}
        )
    # Define properties for Android with Qt 5 after find_package() calls as:
    #    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
else()
    add_executable(Infinity
        ${PROJECT_SOURCES}
    )
endif()
endif()

target_link_libraries(Infinity PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Multimedia Qt${QT_VERSION_MAJOR}::MultimediaWidgets
    "${PROJECT_SOURCE_DIR}/lib/avcodec.lib"
    "${PROJECT_SOURCE_DIR}/lib/avdevice.lib"
    "${PROJECT_SOURCE_DIR}/lib/avfilter.lib"
    "${PROJECT_SOURCE_DIR}/lib/avformat.lib"
    "${PROJECT_SOURCE_DIR}/lib/avutil.lib"
    "${PROJECT_SOURCE_DIR}/lib/postproc.lib"
    "${PROJECT_SOURCE_DIR}/lib/swresample.lib"
    "${PROJECT_SOURCE_DIR}/lib/swscale.lib"
    "${PROJECT_SOURCE_DIR}/lib/SDL2.lib")

set_target_properties(Infinity PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Infinity)
endif()
