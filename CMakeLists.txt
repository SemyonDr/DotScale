cmake_minimum_required(VERSION 4.1.2)

project(dotscale LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
	"${CMAKE_SOURCE_DIR}/out/bin-${CMAKE_BUILD_TYPE}"
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
    "${CMAKE_BINARY_DIR}/lib"
)


add_subdirectory(dotscale_core)