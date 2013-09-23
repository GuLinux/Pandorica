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

void FFMPEGMedia::extractSubtitles()
{

}


FFMPegStreamConversion(AVFormatContext *inputFormatContext, const FFMPEG::Stream &stream)
    : streamIndex(stream.index),
      inputStream(inputFormatContext->streams[stream.index])
{
    decoder = avcodec_find_decoder( inputStream->codec->codec_id );
    // error check: decoder != null
    int result = avcodec_open2( avstream->codec, codec, NULL );
    // error check: result == 0

    pair<string, string> format {"srt", "subrip"};
    result = avformat_alloc_output_context2( &outputFormatContext, NULL, format.first.c_str(), NULL );
    // error check: resut >= 0 && outputFormatContext->oformat != null
}



FFMPEGMedia::FFMPEGMedia( const Media &media, Logger logger )
  : d( media, this )
{
  d->openInputResult = avformat_open_input( &d->pFormatCtx, d->filename, NULL, NULL );

  if( !d->openFileWasValid() ) {
    logger("warning") << "FFMPEGMedia: Unable to open input file '" << media.fullPath() << "'";
    return;
  }
  
  d->findInfoResult = avformat_find_stream_info( d->pFormatCtx, NULL );

  if( !d->findInfoWasValid() ) {
    logger("warning") << "FFMPEGMedia: unable to find info for '" << media.fullPath() << "'";
    return;
  }

  for( int i = 0; i < d->pFormatCtx->nb_streams; i++ )
  {
    d->streams.push_back( d->streamFromAV( d->pFormatCtx->streams[i] ) );
  }
  
  logger("notice") << "Found " << d->streams.size() << " streams in '" << media.fullPath() << "'";

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
  if( d->openFileWasValid() )
    avformat_close_input( &d->pFormatCtx );
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
