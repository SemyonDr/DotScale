
add_library(dotscale_core STATIC
	src/image/ImageBuffer.h
	src/image/ImageBuffer.cpp
	src/image/ImageBuffer_Base.h
	src/image/ImageEnums.h
)

target_compile_features(dotscale_core PUBLIC cxx_std_20)

