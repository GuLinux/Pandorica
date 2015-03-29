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



#ifndef FFMPEGMEDIA_H
#define FFMPEGMEDIA_H
#include "media/media.h"
#include <map>
#include <functional>
#include <ostream>
#include "utils/d_ptr.h"

#ifdef LOGGER_TESTING
#define LOGGER_ENTRY std::ostream&
#else
#define LOGGER_ENTRY Wt::WLogEntry
#endif
#include <memory>
#include <mutex>

namespace boost
{
  class mutex;
}
namespace Wt
{
  class WLogEntry;
}
class Session;

typedef std::vector<uint8_t> BinaryData;
    
namespace FFMPEG
{
  enum StreamType { Video, Audio, Subtitles, Other, All };
  struct Stream
  {
    StreamType type;
    int index;
    std::string title;
    std::pair<int, int> resolution;
    std::map<std::string, std::string> metadata;
    BinaryData data;
    std::string toString() const;
  };
}


class FFMPEGMedia
{
    typedef std::function<LOGGER_ENTRY(const std::string&)> Logger;
  public:
    FFMPEGMedia( const Media &media, Logger logger );
    FFMPEGMedia( const Media &media );
    ~FFMPEGMedia();
    bool isVideo() const;
    std::pair<int, int> resolution() const;
    long durationInSeconds() const;
    bool valid() const;
    std::string metadata( std::string key ) const;
    std::vector<FFMPEG::Stream> streams(FFMPEG::StreamType type = FFMPEG::All) const;
    void extractSubtitles(std::function<void(double)> percentCallback = [](double){});
    void lock();
    void unlock();
    const Media &media() const;
    std::shared_ptr<Image> randomThumbnail(int quality = 100) const;
  private:
    D_PTR;
};

#endif // FFMPEGMEDIA_H
