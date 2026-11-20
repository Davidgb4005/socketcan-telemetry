cmake_minimum_required(VERSION 3.16)
project(socketcan-telemetry VERSION 1.0.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CTest)

add_library(can_telemetry
    src/decoder.cpp
    src/frame.cpp
    src/socketcan.cpp
)
target_include_directories(can_telemetry
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_compile_features(can_telemetry PUBLIC cxx_std_17)
target_compile_options(can_telemetry PRIVATE -Wall -Wextra -Wpedantic -Werror)

add_executable(socketcan-telemetry src/main.cpp)
target_link_libraries(socketcan-telemetry PRIVATE can_telemetry)
target_compile_options(socketcan-telemetry PRIVATE -Wall -Wextra -Wpedantic -Werror)

install(TARGETS socketcan-telemetry RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES packaging/socketcan-telemetry.service
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/systemd/system)

if(BUILD_TESTING)
    add_executable(test_telemetry tests/test_telemetry.cpp)
    target_link_libraries(test_telemetry PRIVATE can_telemetry)
    target_compile_options(test_telemetry PRIVATE -Wall -Wextra -Wpedantic -Werror)
    add_test(NAME telemetry COMMAND test_telemetry)
    add_test(NAME replay
             COMMAND socketcan-telemetry --replay
                     ${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/vehicle.log)
    set_tests_properties(replay PROPERTIES
        PASS_REGULAR_EXPRESSION "voltage_v.*temperature_c")
endif()
