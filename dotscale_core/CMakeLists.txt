
add_library(dotscale_core STATIC
	src/Exceptions.h

	src/image/ImageBuffer.h
	src/image/ImageBuffer.cpp
	src/image/ImageEnums.h

	src/image_io/IOUtil.h
	src/image_io/headers/ImageHeader.h
	src/image_io/headers/PngHeader.h
	src/image_io/headers/JpegHeader.h

	src/image_io/ImageReader.h
	src/image_io/ImageReader.cpp
	src/image_io/ImageWriter.h
	src/image_io/ImageWriter.cpp

	src/image_io/JpegReader.h
	src/image_io/JpegReader.cpp
	

)

target_compile_features(dotscale_core PUBLIC cxx_std_23)

target_link_libraries(dotscale_core
    PRIVATE
        png_static
        JPEG::JPEG
)