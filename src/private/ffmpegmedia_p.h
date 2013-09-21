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




#ifndef FFMPEGMEDIAPRIVATE_H
#define FFMPEGMEDIAPRIVATE_H
#include "media/media.h"
#include "session.h"
#include "ffmpegmedia.h"


extern "C" {
  #include <libavcodec/avcodec.h>    // required headers
  #include <libavformat/avformat.h>
  #include <libavutil/avutil.h>
}

namespace PandoricaPrivate {
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
}
#endif // FFMPEGMEDIAPRIVATE_H
