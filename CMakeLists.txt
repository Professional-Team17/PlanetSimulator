cmake_minimum_required(VERSION 3.10)

project(PlanetSimulator VERSION 1.5)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(PlanetSimulator PlanetSimulator.cpp)

if (CMAKE_HOST_WIN32)
    target_link_libraries(PlanetSimulator mingw32)
endif()
target_link_libraries(PlanetSimulator SDL2main)
target_link_libraries(PlanetSimulator SDL2)
target_link_libraries(PlanetSimulator SDL2_ttf)

set(RESOURCE_FILES font.ttf)
file(COPY ${RESOURCE_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})