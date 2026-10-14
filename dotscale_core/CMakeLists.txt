
add_library(dotscale_core STATIC
	src/image/ImageBuffer.h
	src/image/ImageBuffer.cpp
	src/image/ImageEnums.h
)

target_compile_features(dotscale_core PUBLIC cxx_std_23)

target_link_libraries(dotscale_core
    PRIVATE
        png_static
        JPEG::JPEG
)