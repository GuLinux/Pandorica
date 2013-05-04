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

#ifndef FFMPEGMEDIAPRIVATE_H
#define FFMPEGMEDIAPRIVATE_H
#include "media.h"
#include "session.h"
#include "ffmpegmedia.h"


extern "C" {
  #include <libavcodec/avcodec.h>    // required headers
  #include <libavformat/avformat.h>
  #include <libavutil/avutil.h>
}

class FFMPEGMediaPrivate
{
public:
    FFMPEGMediaPrivate(const Media &media, FFMPEGMedia* q);
    virtual ~FFMPEGMediaPrivate();
    Media media;
    Session *session;
    AVFormatContext *pFormatCtx = 0;
    const char *filename;
    int openInputResult = 0;
    int findInfoResult = 0;
    std::vector<FFMPEG::Stream> streams;
    bool findInfoWasValid() const;
    bool openFileWasValid() const;
    std::map<std::string,std::string> metadata;
    std::map< std::string, std::string > readMetadata(AVDictionary* metadata);
    FFMPEG::Stream streamFromAV(AVStream *stream);
private:
    class FFMPEGMedia* const q;
};

#endif // FFMPEGMEDIAPRIVATE_H
