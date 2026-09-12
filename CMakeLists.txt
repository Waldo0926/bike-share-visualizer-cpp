cmake_minimum_required(VERSION 3.16)
project(bikeshare_visual_monitor_cpp VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(bikeviz_core
    src/CsvStationLoader.cpp
    src/StationRepository.cpp
    src/TripSimulator.cpp
    src/BitmapImage.cpp
)

target_include_directories(bikeviz_core PUBLIC include)

if(MSVC)
    target_compile_options(bikeviz_core PRIVATE /W4)
else()
    target_compile_options(bikeviz_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(bikeviz src/main.cpp)
target_link_libraries(bikeviz PRIVATE bikeviz_core)

if(MSVC)
    target_compile_options(bikeviz PRIVATE /W4)
else()
    target_compile_options(bikeviz PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()
add_executable(bikeviz_tests tests/bikeviz_tests.cpp)
target_link_libraries(bikeviz_tests PRIVATE bikeviz_core)

# Let the integration tests find the bundled dataset and map image
# regardless of where the build directory lives.
target_compile_definitions(bikeviz_tests PRIVATE
    BIKEVIZ_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)

if(MSVC)
    target_compile_options(bikeviz_tests PRIVATE /W4)
else()
    target_compile_options(bikeviz_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(NAME bikeviz_tests COMMAND bikeviz_tests)
