/***********************************************************************
Copyright (c) 2013 "Marco Gulino <marco.gulino@gmail.com>"

This file is part of Pandorica: https://github.com/GuLinux/Pandorica

Pandorica is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details (included the COPYING file).

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/




#include "ffmpegmedia.h"
#include "private/ffmpegmedia_p.h"
#include "utils/d_ptr_implementation.h"
#include <Wt/WLogger>
#include <boost/filesystem.hpp>
#include "utils/utils.h"
#include <libavutil/error.h>
#include <libavutil/mem.h>
#include <boost/chrono/include.hpp>
#include <boost/thread/mutex.hpp>
#ifndef AV_ERROR_MAX_STRING_SIZE
// keep it large, just in case, why not?
#define AV_ERROR_MAX_STRING_SIZE 256
#endif

using namespace std;
using namespace FFMPEG;
namespace fs = boost::filesystem;

FFMPEGMedia::Private::Private( const Media &media, FFMPEGMedia *q )
  : media( media ), q( q ), filename( media.fullPath().c_str() )
{
}



bool FFMPEGMedia::Private::openFileWasValid() const
{
  return openInputResult == 0;
}

bool FFMPEGMedia::Private::findInfoWasValid() const
{
  return openFileWasValid() && findInfoResult >= 0;
}




long FFMPEGMedia::durationInSeconds()
{
  if( !d->findInfoWasValid() )
    return -1;

  auto durationToSecs = [ = ]( uint64_t d )
  {
    return d ? d / AV_TIME_BASE : d;
  };
  uint64_t duration = durationToSecs( d->pFormatCtx->duration );

  return duration;
}


std::string Stream::toString() const
{
  stringstream s;
  static map<StreamType, string> streamTypes
  {
    {StreamType::Video, "video"},
    {StreamType::Audio, "audio"},
    {StreamType::Subtitles, "subtitles"},
    {StreamType::Other, "other/unknown"},
  };
  s << "Stream{ type=" << streamTypes[type] << "index=" << index << ", title=" << title << ", language=" << metadata.at( "language" ) << " }";
  return s.str();
}


void FFMPEGMedia::extractSubtitles( std::function<bool()> keepGoing, function< void( double ) > percentCallback )
{
  boost::unique_lock<boost::mutex> lock( d->mutex );
  list<shared_ptr<FFMPegStreamConversion>> subtitles;

  for( FFMPEG::Stream & stream : d->streams )
  {
    if( stream.type == FFMPEG::Subtitles )
      try
      {
        subtitles.push_back( shared_ptr<FFMPegStreamConversion> {new FFMPegStreamConversion( d->pFormatCtx, stream )} );
      }
      catch
        ( std::exception &e )
      {
        // TODO: better error handling
        d->logger( "error" ) << "error allocating subtitle converter for stream " << stream.toString() << ": " << e.what();
      }
    else
      d->pFormatCtx->streams[stream.index]->discard = AVDISCARD_ALL;
  }

  AVPacket inputPacket;
  av_init_packet( &inputPacket );
  inputPacket.data = NULL;
  inputPacket.size = 0;
  cerr << "reading packets\n";

  int lastPercent = 0;

  while( d->pFormatCtx && keepGoing() && av_read_frame( d->pFormatCtx, &inputPacket ) >= 0 )
  {
    auto packetTimeBase = d->pFormatCtx->streams[inputPacket.stream_index]->time_base;
    int64_t packetSecond = inputPacket.pts * packetTimeBase.num / packetTimeBase.den;
    double percent = packetSecond * 100.0 / ( d->pFormatCtx->duration / AV_TIME_BASE );
    percentCallback( percent );

    for( auto subtitle : subtitles )
      try
      {
        subtitle->addPacket( inputPacket );
      }
      catch
        ( std::exception &e )
      {
        d->logger( "error" ) << "error adding subtitle packet to stream " << subtitle->stream.toString() << ": " << e.what();
      }

    av_free_packet( &inputPacket );
  }
}

#define avLibExec(...) avLibExec2( ConcatStrings({__FILE__, ":",boost::lexical_cast<std::string>(__LINE__)}), __VA_ARGS__)
#define avCreateObject(...) avCreateObject2( ConcatStrings({__FILE__, ":",boost::lexical_cast<std::string>(__LINE__)}), __VA_ARGS__)

void FFMPegStreamConversion::addPacket( AVPacket &inputPacket )
{
  if( inputPacket.stream_index != stream.index )
    return;

  AVSubtitle subtitle;
  int gotSubtitle;
  avLibExec( avcodec_decode_subtitle2( inputStream->codec, &subtitle, &gotSubtitle, &inputPacket ), "decoding subtitle", []( const int & r )
  {
    return r >= 0;
  } );

  if( gotSubtitle )
  {
    av_init_packet( &outputPacket );
    outputPacket.stream_index = outputStream->index;
    int receivedBuffer;
    avLibExec( receivedBuffer = avcodec_encode_subtitle( outputStream->codec, packetsBuffer, PACKETS_BUFFER_SIZE, &subtitle ), "reencoding subtitle", []( const int & r )
    {
      return r >= 0;
    } );
    outputPacket.data = packetsBuffer;
    outputPacket.size = receivedBuffer;
    outputPacket.pts = av_rescale_q( subtitle.pts, AV_TIME_BASE_Q, outputStream->time_base );
    outputPacket.duration = av_rescale_q( subtitle.end_display_time, ( AVRational )
    {
      1, 1000
    }, outputStream->time_base );
    av_write_frame( outputFormatContext, &outputPacket );
  }

  av_free_packet( &outputPacket );
}


FFMPegStreamConversion::FFMPegStreamConversion( AVFormatContext *inputFormatContext, FFMPEG::Stream &stream )
  : stream( stream ),
    inputStream( inputFormatContext->streams[stream.index] )
{
  pair<string, string> encoderFormat {"srt", "subrip"};
  string inputCodecDescription = string( " codec " ) + ( inputStream->codec ? ( inputStream->codec->codec_name + string("-") + boost::lexical_cast<string>(inputStream->codec->codec_id) ): "UNKNOWN" );

  decoder = avCreateObject( avcodec_find_decoder( inputStream->codec->codec_id ), ConcatStrings( {"creating AV Decoder for", inputCodecDescription} ) );
  avLibExec( avcodec_open2( inputStream->codec, decoder, NULL ), ConcatStrings( {"Opening input", inputCodecDescription} ) );

  avLibExec( avformat_alloc_output_context2( &outputFormatContext, NULL, encoderFormat.first.c_str(), NULL ), "allocating output context",
             [this]( const int & r )
  {
    return r >= 0 && outputFormatContext != NULL;
  } );
  encoder = avCreateObject( avcodec_find_encoder_by_name( encoderFormat.second.c_str() ), string( "creating codec " ) + encoderFormat.second );
  outputStream = avCreateObject( avformat_new_stream( outputFormatContext, encoder ), "creating output stream" );
  avcodec_get_context_defaults3( outputStream->codec, encoder );
  outputStream->id = outputFormatContext->nb_streams - 1;
  outputStream->codec->time_base = {1, 1000};

  avLibExec( avio_open_dyn_buf( &outputFormatContext->pb ), "opening output dyn buffer" );
  avLibExec( avformat_write_header( outputFormatContext, NULL ), "writing output format header" );

  vector<uint8_t> subtitleHeader( inputStream->codec->subtitle_header, inputStream->codec->subtitle_header + inputStream->codec->subtitle_header_size );
  subtitleHeader.push_back( 0 );
  outputStream->codec->subtitle_header = subtitleHeader.data();
  outputStream->codec->subtitle_header_size = subtitleHeader.size();

  avLibExec( avcodec_open2( outputStream->codec, encoder, NULL ), "opening output stream" );
}

FFMPegStreamConversion::~FFMPegStreamConversion()
{
  if( outputFormatContext->oformat->write_trailer )
    outputFormatContext->oformat->write_trailer( outputFormatContext );

  av_freep( outputFormatContext->streams[0] );
  uint8_t *output;
  uint64_t outputSize = avio_close_dyn_buf( outputFormatContext->pb, &output );
  stream.data.clear();
  copy( output, output + outputSize, back_insert_iterator<BinaryData>( stream.data ) );
  av_free( output );
}


void FFMPegStreamConversion::avLibExec2( const std::string &where, const int &result, const string &operation, std::function<bool( const int & )> goodCondition )
{
  if( ! goodCondition( result ) )
    throw FFMPEGException( operation + string(" ") + where, result );
}

template<typename T>
T *FFMPegStreamConversion::avCreateObject2( const std::string &where, T *object, const std::string &description )
{
  if( object == NULL )
  {
    throw FFMPEGException( description + string(" ") + where, errno );
  }

  return object;
}


string FFMPEGException::buildErrorMessage( const string &context, int errorCode )
{
  char ffmpegLibraryErrorMessage[AV_ERROR_MAX_STRING_SIZE];
  av_strerror( errorCode, ffmpegLibraryErrorMessage, AV_ERROR_MAX_STRING_SIZE );
  return "Error " + context + ": " + string( ffmpegLibraryErrorMessage );
}



FFMPEGMedia::FFMPEGMedia( const Media &media, Logger logger )
  : d( media, this )
{
  d->logger = logger;
  d->openInputResult = avformat_open_input( &d->pFormatCtx, d->filename, NULL, NULL );

  if( !d->openFileWasValid() )
  {
    logger( "warning" ) << "FFMPEGMedia: Unable to open input file '" << media.fullPath() << "'";
    return;
  }

  d->findInfoResult = avformat_find_stream_info( d->pFormatCtx, NULL );

  if( !d->findInfoWasValid() )
  {
    logger( "warning" ) << "FFMPEGMedia: unable to find info for '" << media.fullPath() << "'";
    return;
  }

  for( int i = 0; i < d->pFormatCtx->nb_streams; i++ )
  {
    d->streams.push_back( d->streamFromAV( d->pFormatCtx->streams[i] ) );
  }

  logger( "notice" ) << "Found " << d->streams.size() << " streams in '" << media.fullPath() << "'";

  d->metadata = d->readMetadata( d->pFormatCtx->metadata );
}


Stream FFMPEGMedia::Private::streamFromAV( AVStream *stream )
{
  StreamType streamType;

  switch( stream->codec->codec_type )
  {
  case AVMEDIA_TYPE_VIDEO:
    streamType = FFMPEG::Video;
    break;

  case AVMEDIA_TYPE_AUDIO:
    streamType = FFMPEG::Audio;
    break;

  case AVMEDIA_TYPE_SUBTITLE:
    streamType = FFMPEG::Subtitles;
    break;

  default
      :
    streamType = FFMPEG::Other;
  }

  auto streamMetadata = readMetadata( stream->metadata );
  pair<int, int> resolution = ( streamType == FFMPEG::Video ) ? pair<int, int> {stream->codec->width, stream->codec->height} :
                              pair<int, int> { -1, -1};
  return {streamType, stream->index, streamMetadata["title"], resolution, streamMetadata};
}


map<string, string> FFMPEGMedia::Private::readMetadata( AVDictionary *metadata )
{
  AVDictionaryEntry *entry = NULL;
  map<string, string> result;

  while( entry = av_dict_get( metadata, "", entry, AV_DICT_IGNORE_SUFFIX ) )
    result[entry->key] = entry->value;

  return result;
}


FFMPEGMedia::~FFMPEGMedia()
{
  boost::unique_lock<boost::mutex> lock( d->mutex );

  if( d->openFileWasValid() )
  {
    avformat_close_input( &d->pFormatCtx );
  }

  d->pFormatCtx = 0;
}

bool FFMPEGMedia::valid()
{
  return d->openFileWasValid() && d->findInfoWasValid();
}


std::string FFMPEGMedia::metadata( std::string key ) const
{
  return d->metadata[key];
}

std::vector<Stream> FFMPEGMedia::streams() const
{
  return d->streams;
}


bool FFMPEGMedia::isVideo()
{
  for( auto stream : d->streams )
    if( stream.type == Video )
      return true;

  return false;
}

std::pair<int, int> FFMPEGMedia::resolution()
{
  if( !isVideo() )
    return { -1, -1};

  for( auto stream : d->streams )
    if( stream.type == Video )
      return stream.resolution;
}
