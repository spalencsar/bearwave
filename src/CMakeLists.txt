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
    icyreader.cpp
    systemtraymanager.cpp
)

# Create executable
add_executable(bearwave ${SOURCES})

if(BEARWAVE_GIT_HASH)
    # set via -D from build scripts
elseif(DEFINED ENV{BEARWAVE_GIT_HASH} AND NOT "$ENV{BEARWAVE_GIT_HASH}" STREQUAL "")
    set(BEARWAVE_GIT_HASH "$ENV{BEARWAVE_GIT_HASH}")
else()
    execute_process(
        COMMAND git rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..
        OUTPUT_VARIABLE BEARWAVE_GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
endif()
if(NOT BEARWAVE_GIT_HASH)
    set(BEARWAVE_GIT_HASH "unknown")
endif()

target_compile_definitions(bearwave PRIVATE
    BEARWAVE_VERSION="${PROJECT_VERSION}"
    BEARWAVE_GIT_HASH="${BEARWAVE_GIT_HASH}"
)

# Qt Resources
target_sources(bearwave PRIVATE qml.qrc)

qt_add_lrelease(bearwave
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
    Qt6::Multimedia
)

# Install
install(TARGETS bearwave DESTINATION bin)
