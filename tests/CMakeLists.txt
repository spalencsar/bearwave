find_package(Qt6 REQUIRED COMPONENTS Test)

set(TEST_SOURCES
    radiobackend_playback_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/radiobackend.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/radiostation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/radiobrowser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/bearplayer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/coverartfetcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/icyreader.cpp
)

add_executable(radiobackend_playback_test ${TEST_SOURCES})

target_include_directories(radiobackend_playback_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(radiobackend_playback_test PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Test
    Qt6::Widgets
    Qt6::Multimedia
)

add_test(NAME radiobackend_playback_test COMMAND radiobackend_playback_test)

set(RACE_TEST_SOURCES
    radiobrowser_race_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/radiobrowser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/radiostation.cpp
)

add_executable(radiobrowser_race_test ${RACE_TEST_SOURCES})

target_include_directories(radiobrowser_race_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(radiobrowser_race_test PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Test
    Qt6::Widgets
)

add_test(NAME radiobrowser_race_test COMMAND radiobrowser_race_test)