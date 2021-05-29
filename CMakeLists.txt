cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(PlanetSimulator VERSION 1.5)

add_executable(PlanetSimulator PlanetSimulator.cpp)

if (MINGW)
    target_link_libraries(PlanetSimulator mingw32 SDL2main SDL2 SDL2_ttf)
endif()
if(MSVC)
    find_package(SDL2 CONFIG REQUIRED)
    find_package(sdl2-ttf CONFIG REQUIRED)
    target_link_libraries(PlanetSimulator SDL2::SDL2main SDL2::SDL2 SDL2::SDL2_ttf)
endif()

set(RESOURCE_FILES font.ttf)
file(COPY ${RESOURCE_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})