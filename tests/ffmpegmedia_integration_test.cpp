#include "boost_unit_tests_helper.h"
#include <ffmpegmedia.h>
#include <sstream>
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

struct Setup
{
  stringstream warnings;
  stringstream notices;
  map<string,stringstream*> logs {
    {"warning", &warnings},
    {"notice", &notices },
  };
  ostream& logger(const string& level) { return *logs[level]; }
};

BOOST_FIXTURE_TEST_CASE( TestEmptyFFMPegMedia, Setup )
{
  FFMPEGMedia media( Media {},  bind(&Setup::logger, this, placeholders::_1) );
  BOOST_REQUIRE( !media.valid() );
  BOOST_REQUIRE_EQUAL( 0, media.streams().size() );
  BOOST_REQUIRE_EQUAL( string{"FFMPEGMedia: Unable to open input file ''"}, warnings.str());
}

BOOST_FIXTURE_TEST_CASE( TestInvalidMedia, Setup )
{
  string mediaFile = string{FFMPEG_MEDIA_INTEGRATION_TESTS_SAMPLES_DIR} + "/not_a_real_video.mp4";
  FFMPEGMedia media( Media {mediaFile},  bind(&Setup::logger, this, placeholders::_1) );
  BOOST_REQUIRE( !media.valid() );
  BOOST_REQUIRE_EQUAL( 0, media.streams().size() );
  BOOST_REQUIRE_EQUAL( string{"FFMPEGMedia: Unable to open input file '"} + mediaFile + "'", warnings.str());
}


BOOST_FIXTURE_TEST_CASE( TestValidVideo, Setup )
{
  FFMPEGMedia media( Media {string{FFMPEG_MEDIA_INTEGRATION_TESTS_SAMPLES_DIR} + "/sample_mpeg4.mp4" },  bind(&Setup::logger, this, placeholders::_1) );
  BOOST_REQUIRE( media.valid() );
  BOOST_REQUIRE_EQUAL( 2, media.streams().size() );
  BOOST_REQUIRE_EQUAL( FFMPEG::Video, media.streams()[1].type );
  BOOST_REQUIRE_EQUAL( FFMPEG::Audio, media.streams()[0].type );
  BOOST_REQUIRE_EQUAL( "Sample MP4 Video", media.metadata( "title" ) );
  BOOST_REQUIRE_EQUAL( "", warnings.str());
}


BOOST_FIXTURE_TEST_CASE( TestValidVideoWithSubtitles, Setup )
{
  FFMPEGMedia media( Media {string{FFMPEG_MEDIA_INTEGRATION_TESTS_SAMPLES_DIR} + "/sample_mpeg4_with_subtitle.mp4" },  bind(&Setup::logger, this, placeholders::_1) );
  BOOST_REQUIRE( media.valid() );
  BOOST_REQUIRE_EQUAL( 3, media.streams().size() );
  BOOST_REQUIRE_EQUAL( FFMPEG::Video, media.streams()[1].type );
  BOOST_REQUIRE_EQUAL( FFMPEG::Audio, media.streams()[0].type );
  BOOST_REQUIRE_EQUAL( FFMPEG::Subtitles, media.streams()[2].type );
  BOOST_REQUIRE_EQUAL( "Sample MP4 Video With Subtitles", media.metadata( "title" ) );
  BOOST_REQUIRE_EQUAL( "", warnings.str());
}
