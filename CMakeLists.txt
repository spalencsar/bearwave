cmake_minimum_required(VERSION 3.16)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(bearwave VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt6
find_package(Qt6 REQUIRED COMPONENTS Core DBus Quick QuickControls2 Widgets)

# KDE Frameworks
find_package(KF6Kirigami REQUIRED)
find_package(KF6I18n REQUIRED)
find_package(KF6CoreAddons REQUIRED)

# Phonon (Qt6)
find_package(Phonon4Qt6 REQUIRED)

# Add src directory
add_subdirectory(src)

# Configure and install desktop file with correct binary path
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/org.kde.bearwave.desktop.in
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.bearwave.desktop
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.bearwave.desktop DESTINATION share/applications)

# Install app icon
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/app/bearwave.png DESTINATION share/icons/hicolor/256x256/apps RENAME org.kde.bearwave.png)
