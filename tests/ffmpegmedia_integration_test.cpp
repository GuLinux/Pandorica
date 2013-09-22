#include "boost_unit_tests_helper.h"
#include <ffmpegmedia.h>
using namespace std;
extern "C" {
#include <libavcodec/avcodec.h>    // required headers
#include <libavformat/avformat.h>
}

struct InitFFMPeg
{
    InitFFMPeg()
    {
      av_register_all();
    }
};

InitFFMPeg initFFMPEG;


BOOST_AUTO_TEST_CASE(TestEmptyFFMPegMedia)
{
  FFMPEGMedia media(Media{});
  BOOST_REQUIRE( !media.valid() );
  BOOST_REQUIRE_EQUAL(0, media.streams().size() );
}



BOOST_AUTO_TEST_CASE(TestInvalidMedia)
{
  FFMPEGMedia media(Media{string{FFMPEG_MEDIA_INTEGRATION_TESTS_SAMPLES_DIR} + "/not_a_real_video.mp4" });
  BOOST_REQUIRE( !media.valid() );
  BOOST_REQUIRE_EQUAL(0, media.streams().size() );
}


BOOST_AUTO_TEST_CASE(TestValidVideo)
{
  FFMPEGMedia media(Media{string{FFMPEG_MEDIA_INTEGRATION_TESTS_SAMPLES_DIR} + "/sample_mpeg4.mp4" });
  BOOST_REQUIRE( media.valid() );
  BOOST_REQUIRE_EQUAL(2, media.streams().size() );
  BOOST_REQUIRE_EQUAL( FFMPEG::Video, media.streams()[1].type);
  BOOST_REQUIRE_EQUAL( FFMPEG::Audio, media.streams()[0].type);
  BOOST_REQUIRE_EQUAL( "Sample MP4 Video", media.metadata("title") );
}


BOOST_AUTO_TEST_CASE(TestValidVideoWithSubtitles)
{
  FFMPEGMedia media(Media{string{FFMPEG_MEDIA_INTEGRATION_TESTS_SAMPLES_DIR} + "/sample_mpeg4_with_subtitle.mp4" });
  BOOST_REQUIRE( media.valid() );
  BOOST_REQUIRE_EQUAL(3, media.streams().size() );
  BOOST_REQUIRE_EQUAL( FFMPEG::Video, media.streams()[1].type);
  BOOST_REQUIRE_EQUAL( FFMPEG::Audio, media.streams()[0].type);
  BOOST_REQUIRE_EQUAL( FFMPEG::Subtitles, media.streams()[2].type);
  BOOST_REQUIRE_EQUAL( "Sample MP4 Video With Subtitles", media.metadata("title") );
}