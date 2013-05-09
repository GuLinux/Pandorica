/*
 * Copyright 2013 Marco Gulino <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "ffmpegmedia.h"
#include "ffmpegmedia_p.h"

using namespace std;
using namespace FFMPEG;
using namespace StreamingPrivate;

FFMPEGMediaPrivate::FFMPEGMediaPrivate(const Media& media, FFMPEGMedia* q) : media(media), q(q), filename(media.fullPath().c_str())
{
}



FFMPEGMediaPrivate::~FFMPEGMediaPrivate()
{
}

bool FFMPEGMediaPrivate::openFileWasValid() const
{
  return openInputResult == 0;
}

bool FFMPEGMediaPrivate::findInfoWasValid() const
{
  return openFileWasValid() && findInfoResult >= 0;
}




long FFMPEGMedia::durationInSeconds()
{
  if(!d->findInfoWasValid())
    return -1;
  auto durationToSecs = [=](uint64_t d) { return d ? d/AV_TIME_BASE:d; };
  uint64_t duration = durationToSecs(d->pFormatCtx->duration);

  return duration;
}


FFMPEGMedia::FFMPEGMedia(const Media& media)
    : d(new FFMPEGMediaPrivate(media, this))
{
  d->openInputResult = avformat_open_input(&d->pFormatCtx, d->filename, NULL, NULL); 
  if( d->openFileWasValid() )
    d->findInfoResult = avformat_find_stream_info(d->pFormatCtx, NULL);
  if(!d->findInfoWasValid())
    return;
  for(int i=0; i<d->pFormatCtx->nb_streams; i++) {
    d->streams.push_back(d->streamFromAV(d->pFormatCtx->streams[i]));
  }
  d->metadata = d->readMetadata(d->pFormatCtx->metadata);
}


Stream FFMPEGMediaPrivate::streamFromAV(AVStream* stream)
{
  StreamType streamType;
  switch(stream->codec->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
      streamType = FFMPEG::Video;
      break;
    case AVMEDIA_TYPE_AUDIO:
      streamType = FFMPEG::Audio;
      break;
    case AVMEDIA_TYPE_SUBTITLE:
      streamType = FFMPEG::Subtitles;
      break;
    default:
      streamType = FFMPEG::Other;
  }
  auto streamMetadata = readMetadata(stream->metadata);
  pair<int,int> resolution = (streamType==FFMPEG::Video) ? pair<int,int>{stream->codec->width, stream->codec->height} : pair<int,int>{-1,-1};
  return {streamType, stream->index, streamMetadata["title"], resolution, streamMetadata};
}


map<string,string> FFMPEGMediaPrivate::readMetadata(AVDictionary *metadata)
{
  AVDictionaryEntry *entry = NULL;
  map<string,string> result;
  while (entry = av_dict_get(metadata, "", entry, AV_DICT_IGNORE_SUFFIX))
    result[entry->key] = entry->value;
  return result;
}


FFMPEGMedia::~FFMPEGMedia()
{
  if(d->openFileWasValid())
    avformat_close_input(&d->pFormatCtx);
  delete d;
}

bool FFMPEGMedia::valid()
{
  return d->openFileWasValid() && d->findInfoWasValid(); 
}


std::string FFMPEGMedia::metadata(std::string key) const
{
  return d->metadata[key];
}

std::vector<Stream> FFMPEGMedia::streams() const
{
  return d->streams;
}


bool FFMPEGMedia::isVideo()
{
  for(auto stream: d->streams)
    if(stream.type == Video)
      return true;
  return false;
}

std::pair<int,int> FFMPEGMedia::resolution()
{
  if(!isVideo())
    return {-1, -1};
  for(auto stream: d->streams)
    if(stream.type == Video)
      return stream.resolution;
}
