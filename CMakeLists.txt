cmake_minimum_required(VERSION 3.21)
project(NEON_CITY LANGUAGES C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(EMSCRIPTEN)
    include(FetchContent)
    set(PLATFORM Web CACHE STRING "" FORCE)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 6.0
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(raylib)
else()
    find_package(raylib REQUIRED)
endif()

add_executable(neon_city
    src/main.c src/menu.c src/player.c src/map.c src/hub_terminal.c
    src/combat.c src/enemy.c src/mission.c src/economy.c src/shop.c
    src/savegame.c src/city_visuals.c src/city_hud.c)
target_include_directories(neon_city PRIVATE include)
target_link_libraries(neon_city PRIVATE raylib)

if(EMSCRIPTEN)
    set_target_properties(neon_city PROPERTIES OUTPUT_NAME index SUFFIX ".html")
    target_compile_options(neon_city PRIVATE -O2)
    target_link_options(neon_city PRIVATE
        -O2 -sUSE_GLFW=3 -sASYNCIFY -sALLOW_MEMORY_GROWTH=1
        -sFORCE_FILESYSTEM=1 -sEXIT_RUNTIME=1 -sSTACK_SIZE=1048576
        "--shell-file=${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html"
        "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/data@data"
        "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/assets/audio@assets/audio")
    set_property(TARGET neon_city APPEND PROPERTY LINK_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html"
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/audio/bass_over.wav"
        "${CMAKE_CURRENT_SOURCE_DIR}/data/central_hub.map"
        "${CMAKE_CURRENT_SOURCE_DIR}/data/training_arena.map")
elseif(APPLE)
    target_link_libraries(neon_city PRIVATE
        "-framework IOKit" "-framework Cocoa" "-framework OpenGL")
endif()
