include(FetchContent)

# ------------------------------------------------------------
# zlib
# ------------------------------------------------------------

set(ZLIB_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(ZLIB_BUILD_STATIC ON  CACHE BOOL "" FORCE)
set(ZLIB_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(ZLIB_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    zlib
    GIT_REPOSITORY https://github.com/madler/zlib.git
    GIT_TAG        v1.3.2
    OVERRIDE_FIND_PACKAGE
)

FetchContent_MakeAvailable(zlib)

if(NOT TARGET ZLIB::ZLIB)
    add_library(ZLIB::ZLIB ALIAS zlibstatic)
endif()

# ------------------------------------------------------------
# libpng
# ------------------------------------------------------------

set(PNG_SHARED OFF CACHE BOOL "" FORCE)
set(PNG_STATIC ON  CACHE BOOL "" FORCE)
set(PNG_TESTS  OFF CACHE BOOL "" FORCE)
set(PNG_TOOLS  OFF CACHE BOOL "" FORCE)
set(SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    libpng
    GIT_REPOSITORY https://github.com/pnggroup/libpng.git
    GIT_TAG        v1.6.59
)

FetchContent_MakeAvailable(libpng)


# ------------------------------------------------------------
# libjpeg-turbo
# ------------------------------------------------------------

include(ExternalProject)

set(JPEG_DIR "${CMAKE_BINARY_DIR}/_deps/libjpeg-turbo")
set(JPEG_INSTALL "${JPEG_DIR}/install")
set(JPEG_LIBRARY "${JPEG_INSTALL}/lib/jpeg-static.lib")

# Imported targets require include directories to exist
# when CMake configures the main project.
file(MAKE_DIRECTORY "${JPEG_INSTALL}/include")

ExternalProject_Add(libjpeg_turbo_external
    GIT_REPOSITORY https://github.com/libjpeg-turbo/libjpeg-turbo.git
    GIT_TAG 3.2.0

    PREFIX "${JPEG_DIR}"

    CMAKE_GENERATOR "${CMAKE_GENERATOR}"

    CMAKE_ARGS
        "-DCMAKE_INSTALL_PREFIX=${JPEG_INSTALL}"
        "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
        "-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}"
        "-DCMAKE_INSTALL_LIBDIR=lib"

        "-DENABLE_SHARED=OFF"
        "-DENABLE_STATIC=ON"
        "-DWITH_TOOLS=OFF"
        "-DWITH_TESTS=OFF"
        "-DWITH_TURBOJPEG=OFF"
        "-DWITH_CRT_DLL=ON"

    INSTALL_BYPRODUCTS "${JPEG_LIBRARY}"
)

add_library(JPEG::JPEG STATIC IMPORTED GLOBAL)

set_target_properties(JPEG::JPEG PROPERTIES
    IMPORTED_LOCATION "${JPEG_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${JPEG_INSTALL}/include"
)

add_dependencies(JPEG::JPEG libjpeg_turbo_external)