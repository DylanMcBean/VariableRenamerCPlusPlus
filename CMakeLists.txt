cmake_minimum_required(VERSION 3.5)

project(VariableChangerGui VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

# Add the qss file to the project
set(QSS_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Stylesheets/dark_mode.qss
    ${CMAKE_CURRENT_SOURCE_DIR}/Stylesheets/dark_mode_v2.qss
    ${CMAKE_CURRENT_SOURCE_DIR}/Stylesheets/dark_mode_modern.qss
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(VariableChangerGui
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(VariableChangerGui SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(VariableChangerGui
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(VariableChangerGui PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(VariableChangerGui PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Copy the qss file to the build folder
foreach(QSS_FILE ${QSS_FILES})
    configure_file(${QSS_FILE} ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
endforeach()

install(TARGETS VariableChangerGui
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(VariableChangerGui)
endif()
