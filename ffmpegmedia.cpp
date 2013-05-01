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
  for(int i=0; i<d->pFormatCtx->nb_streams; i++)
    d->streams.push_back(d->pFormatCtx->streams[i]);
  d->readMetadata();
}



void FFMPEGMediaPrivate::readMetadata()
{
  AVDictionary *metadata = pFormatCtx->metadata;
  AVDictionaryEntry *entry = NULL;
  while (entry = av_dict_get(metadata, "", entry, AV_DICT_IGNORE_SUFFIX))
    this->metadata[entry->key] = entry->value;
}


FFMPEGMedia::~FFMPEGMedia()
{
  avformat_close_input(&d->pFormatCtx);
  delete d;
}

std::string FFMPEGMedia::metadata(std::string key) const
{
  return d->metadata[key];
}


bool FFMPEGMedia::isVideo()
{
  for(auto stream: d->streams)
    if(stream->codec->codec_type == AVMEDIA_TYPE_VIDEO)
      return true;
  return false;
}

std::pair< int, int > FFMPEGMedia::resolution()
{
  if(!isVideo())
    return {-1, -1};
  for(auto stream: d->streams)
    if(stream->codec->codec_type == AVMEDIA_TYPE_VIDEO)
      return {stream->codec->width, stream->codec->height};
}
