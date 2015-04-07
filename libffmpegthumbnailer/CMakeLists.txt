SET( libffmpegthumbnailer_srcs
    videothumbnailer.cpp
    filmstripfilter.cpp
    pngwriter.cpp
    jpegwriter.cpp
    moviedecoder.cpp
)
include_directories(${AVUTIL_INCLUDE_DIRS} ${AVFORMAT_INCLUDE_DIRS} ${AVCODEC_INCLUDE_DIRS} ${SWSCALE_INCLUDE_DIRS} ${LIBPNG_INCLUDE_DIRS})

add_library(pandorica_libffmpegthumbnailer STATIC ${libffmpegthumbnailer_srcs})