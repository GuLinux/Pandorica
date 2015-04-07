SET( libffmpegthumbnailer_srcs
    videothumbnailer.cpp
    filmstripfilter.cpp
    pngwriter.cpp
    jpegwriter.cpp
    moviedecoder.cpp
)

add_library(pandorica_libffmpegthumbnailer STATIC ${libffmpegthumbnailer_srcs})