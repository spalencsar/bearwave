cmake_minimum_required(VERSION 3.16)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(bearwave VERSION 1.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt6
find_package(Qt6 REQUIRED COMPONENTS Core DBus Quick QuickControls2 Widgets LinguistTools Multimedia)

# Add src directory
add_subdirectory(src)

option(BEARWAVE_BUILD_TESTS "Build BearWave unit tests" ON)
if(BEARWAVE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Configure and install desktop file with correct binary path
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/de.nerdbear.bearwave.desktop.in
    ${CMAKE_CURRENT_BINARY_DIR}/de.nerdbear.bearwave.desktop
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/de.nerdbear.bearwave.desktop DESTINATION share/applications)

# Install app icon
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/app/bearwave.svg DESTINATION share/icons/hicolor/scalable/apps RENAME de.nerdbear.bearwave.svg)

# Install AppStream metadata
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/de.nerdbear.bearwave.metainfo.xml DESTINATION share/metainfo)
