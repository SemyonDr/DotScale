cmake_minimum_required(VERSION 4.1.2)


add_library(dotscale_core STATIC
	src/image/ImageBuffer.h
	src/image/ImageBuffer.cpp
	src/image/ImageBuffer_Base.h
	src/image/ImageEnums.h
)

target_compile_features(dotscale_core PUBLIC cxx_std_20)

target_include_directories(dotscale_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)