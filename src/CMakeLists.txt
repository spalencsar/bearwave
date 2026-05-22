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
    notificationmanager.cpp
)

# Create executable
add_executable(bearwave ${SOURCES})

# Qt Resources
target_sources(bearwave PRIVATE qml.qrc)

qt_add_lrelease(
    TS_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../translations/bearwave_de.ts
    QM_FILES_OUTPUT_VARIABLE bearwave_qm_files
)

foreach(qm_file IN LISTS bearwave_qm_files)
    get_filename_component(qm_name "${qm_file}" NAME)
    set_source_files_properties("${qm_file}" PROPERTIES QT_RESOURCE_ALIAS "${qm_name}")
endforeach()

qt_add_resources(bearwave bearwave_translations
    PREFIX /i18n
    FILES ${bearwave_qm_files}
)

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
