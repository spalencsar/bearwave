# BearWave Source Files

# Core application
set(SOURCES
    main.cpp
    radiostation.cpp
    radiobrowser.cpp
    radiobackend.cpp
    bearplayer.cpp
    coverartfetcher.cpp
    mprisadaptor.cpp
    bearwavecontroladaptor.cpp
)

# Create executable
add_executable(bearwave ${SOURCES})

# Qt Resources
target_sources(bearwave PRIVATE qml.qrc)

# Link libraries
target_link_libraries(bearwave PRIVATE
    Qt6::Core
    Qt6::DBus
    Qt6::Network
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Widgets
    KF6::Kirigami
    KF6::I18n
    KF6::CoreAddons
    Phonon::phonon4qt6
)

# Install
install(TARGETS bearwave DESTINATION bin)
