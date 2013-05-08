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

#ifndef FFMPEGMEDIA_H
#define FFMPEGMEDIA_H
#include "media.h"
#include <map>

class Session;

namespace FFMPEG {
  enum StreamType { Video, Audio, Subtitles, Other };
  struct Stream {
    StreamType type;
    int index;
    std::string title;
    std::pair<int,int> resolution;
    std::map<std::string,std::string> metadata;
  };
};

class FFMPEGMedia
{
public:
  FFMPEGMedia(const Media &media);
  ~FFMPEGMedia();
  bool isVideo();
  std::pair<int,int> resolution();
  long durationInSeconds();
  bool valid();
  std::string metadata(std::string key) const;
  std::vector<FFMPEG::Stream> streams() const;
private:
    class FFMPEGMediaPrivate* const d;
};

#endif // FFMPEGMEDIA_H
