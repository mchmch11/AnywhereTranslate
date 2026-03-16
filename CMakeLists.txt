cmake_minimum_required(VERSION 3.16)
project(AnywhereTranslate VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_PREFIX_PATH "D:/Software/Qt/6.10.1/mingw_64")
find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

qt_add_executable(AnywhereTranslate WIN32
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/TranslatorService.h
    src/TranslatorService.cpp
    src/SettingsDialog.h
    src/SettingsDialog.cpp
    src/GlobalHotkeyWatcher.h
    src/GlobalHotkeyWatcher.cpp
    src/PopupWindow.h
    src/PopupWindow.cpp
)

target_link_libraries(AnywhereTranslate PRIVATE Qt6::Widgets Qt6::Network)

install(TARGETS AnywhereTranslate
    RUNTIME DESTINATION bin
)
