cmake_minimum_required(VERSION 3.20)

project(MyGame LANGUAGES C CXX)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL_CPACK OFF CACHE BOOL "" FORCE)

file(
    GLOB_RECURSE GAME_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.c"
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

add_executable(
    ${PROJECT_NAME}
    ${GAME_SOURCES}
)

target_compile_definitions(
    ${PROJECT_NAME}
    PRIVATE
        BUILD_TYPE_DEBUG=0
        BUILD_TYPE_RELWITHDEBINFO=1
        BUILD_TYPE_RELEASE=2
        BUILD_TYPE_MINSIZEREL=3
        PROJECT_NAME="${PROJECT_NAME}"
)
target_compile_definitions(
    ${PROJECT_NAME}
    PRIVATE
        BUILD_TYPE=$<IF:$<CONFIG:Debug>,0,$<IF:$<CONFIG:RelWithDebInfo>,1,$<IF:$<CONFIG:Release>,2,$<IF:$<CONFIG:MinSizeRel>,3,-1>>>>
        BUILD_TYPE_STR="$<CONFIG>"
)
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src/")

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.2.22
    GIT_SHALLOW TRUE
)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_SHALLOW TRUE
)
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG        658f48e72aee3c6582e80b05ac0f8787a64fe6bb #v2.0.6
    GIT_PROGRESS   TRUE
    SOURCE_SUBDIR  cmake
)
FetchContent_Declare(
    cmakerc
    GIT_REPOSITORY https://github.com/vector-of-bool/cmrc.git
    GIT_TAG        2.0.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(SDL3 glad cmakerc glm)

file(
    GLOB_RECURSE GAME_ASSETS
    CONFIGURE_DEPENDS
    RELATIVE "${CMAKE_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/assets/*"
)

cmrc_add_resource_library(
    game_assets
    NAMESPACE assets
    WHENCE "${CMAKE_SOURCE_DIR}"
    ${GAME_ASSETS}
)

glad_add_library(glad_gl_core_43 STATIC REPRODUCIBLE LOADER API gl:core=4.3)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
        SDL3::SDL3-static
        glad_gl_core_43
        game_assets
        glm::glm
)

if(NOT EMSCRIPTEN)
    find_package(OpenGL REQUIRED)

    target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
            OpenGL::GL
    )
endif()

if(WIN32)
    target_link_options(MyGame PRIVATE
        -static-libgcc
        -static-libstdc++
    )

    set_target_properties(
        ${PROJECT_NAME}
        PROPERTIES
            WIN32_EXECUTABLE TRUE
    )
endif()

if(EMSCRIPTEN)
    set_target_properties(
        ${PROJECT_NAME}
        PROPERTIES
            SUFFIX ".html"
    )

    target_compile_options(
        ${PROJECT_NAME}
        PRIVATE
            "-fexceptions"
    )

    target_link_options(
        ${PROJECT_NAME}
        PRIVATE
            "-sUSE_WEBGL2=1"
            "-sALLOW_MEMORY_GROWTH=1"
            "-sWASM=1"
            "-fexceptions"
            "-sASSERTIONS=1"
            "--shell-file"
            "${CMAKE_SOURCE_DIR}/web/shell.html"
    )
endif()
