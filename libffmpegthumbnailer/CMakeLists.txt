SET( libffmpegthumbnailer_srcs
    videothumbnailer.cpp
    filmstripfilter.cpp
    pngwriter.cpp
    jpegwriter.cpp
    moviedecoder.cpp
)
include_directories(${FFMPEG_STATIC_INCLUDES})

add_library(pandorica_libffmpegthumbnailer STATIC ${libffmpegthumbnailer_srcs})
add_dependencies(pandorica_libffmpegthumbnailer ffmpeg_libraries)
