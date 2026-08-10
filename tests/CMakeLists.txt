find_package(Qt6 REQUIRED COMPONENTS Test)

set(BEARWAVE_TEST_ENV
    QT_QPA_PLATFORM=offscreen
    QT_MEDIA_BACKEND=ffmpeg
)

set(TEST_SOURCES
    radiobackend_playback_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/radiobackend.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/radiostation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/countrynames.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/stationimagecache.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/stationartworkservice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/stationlogostyle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/radiobrowser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/bearplayer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/nowplayingstate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/coverartfetcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/icyreader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/qml.qrc
)

add_executable(radiobackend_playback_test ${TEST_SOURCES})

target_include_directories(radiobackend_playback_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

# radiobrowser.cpp uses BEARWAVE_VERSION in the Radio Browser User-Agent.
target_compile_definitions(radiobackend_playback_test PRIVATE
    BEARWAVE_VERSION="${PROJECT_VERSION}"
)

target_link_libraries(radiobackend_playback_test PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Test
    Qt6::Widgets
    Qt6::Multimedia
)

add_test(NAME radiobackend_playback_test COMMAND radiobackend_playback_test)
set_tests_properties(radiobackend_playback_test PROPERTIES ENVIRONMENT "${BEARWAVE_TEST_ENV}")

set(RACE_TEST_SOURCES
    radiobrowser_race_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/radiobrowser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/radiostation.cpp
)

add_executable(radiobrowser_race_test ${RACE_TEST_SOURCES})

target_include_directories(radiobrowser_race_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_compile_definitions(radiobrowser_race_test PRIVATE
    BEARWAVE_VERSION="${PROJECT_VERSION}"
)

target_link_libraries(radiobrowser_race_test PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Test
)

add_test(NAME radiobrowser_race_test COMMAND radiobrowser_race_test)
set_tests_properties(radiobrowser_race_test PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

add_executable(qrc_resources_test
    qrc_resources_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/qml.qrc
)

target_link_libraries(qrc_resources_test PRIVATE Qt6::Core Qt6::Test)

add_test(NAME qrc_resources_test COMMAND qrc_resources_test)

add_executable(country_names_test
    country_names_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/countrynames.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/qml.qrc
)

target_include_directories(country_names_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(country_names_test PRIVATE Qt6::Core Qt6::Test)

add_test(NAME country_names_test COMMAND country_names_test)

add_executable(appsettings_test
    appsettings_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/appsettings.cpp
)

target_include_directories(appsettings_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(appsettings_test PRIVATE Qt6::Core Qt6::Test)

add_test(NAME appsettings_test COMMAND appsettings_test)

add_executable(station_image_cache_test
    station_image_cache_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/stationimagecache.cpp
)

target_include_directories(station_image_cache_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(station_image_cache_test PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Network
    Qt6::Test
)

add_test(NAME station_image_cache_test COMMAND station_image_cache_test)

add_executable(station_artwork_service_test
    station_artwork_service_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/stationartworkservice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/stationimagecache.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/stationlogostyle.cpp
)

target_include_directories(station_artwork_service_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(station_artwork_service_test PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Network
    Qt6::Test
)

add_test(NAME station_artwork_service_test COMMAND station_artwork_service_test)
