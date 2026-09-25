cmake_minimum_required(VERSION 3.20)

project(MyGame LANGUAGES C CXX)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(GL_VERSION_MAJOR 4)
set(GL_VERSION_MINOR 3)

include(FetchContent)

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

find_package(SDL3 CONFIG REQUIRED COMPONENTS SDL3-static)
find_package(glm CONFIG REQUIRED)

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
FetchContent_MakeAvailable(glad cmakerc)

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

glad_add_library(
    glad_gl_core
    STATIC
    REPRODUCIBLE
    LOADER
    API gl:core=${GL_VERSION_MAJOR}.${GL_VERSION_MINOR}
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
        SDL3::SDL3-static
        glad_gl_core
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
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        GL_VERSION_MAJOR=3
        GL_VERSION_MINOR=0
    )
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        GL_VERSION_MAJOR=${GL_VERSION_MAJOR}
        GL_VERSION_MINOR=${GL_VERSION_MINOR}
    )
endif()
