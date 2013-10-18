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
#include <boost/thread/pthread/mutex.hpp>


extern "C" {
#include <libavcodec/avcodec.h>    // required headers
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}


#define PACKETS_BUFFER_SIZE 200000

class FFMPEGException : public std::runtime_error
{
  public:
    explicit FFMPEGException( const std::string &context, int errorCode )
      : runtime_error( FFMPEGException::buildErrorMessage( context, errorCode ) ) {}
  private:
    static std::string buildErrorMessage( const std::string &context, int errorCode );
};

namespace
{
  struct FFMPegStreamConversion
  {
    FFMPegStreamConversion( AVFormatContext *inputFormatContext, FFMPEG::Stream &stream );
    ~FFMPegStreamConversion();
    void avLibExec2( const std::string &where, const int &result, const std::string &operation, std::function<bool( const int & )> goodCondition = []( const int &r )
    {
      return r == 0;
    } );
    template<typename T>
    T *avCreateObject2( const std::string &where, T *object, const std::string &description );
    void addPacket( AVPacket &inputPacket );
    FFMPEG::Stream &stream;
    AVStream *inputStream;
    AVStream *outputStream;
    AVCodec *decoder;
    AVFormatContext *outputFormatContext;
    AVCodec *encoder;
    AVPacket outputPacket;


    uint8_t packetsBuffer[PACKETS_BUFFER_SIZE];
  };
}

class FFMPEGMedia::Private
{
  public:
    Private( const Media &media, FFMPEGMedia *q );
    Media media;
    Session *session;
    AVFormatContext *pFormatCtx = 0;
    const char *filename;
    int openInputResult = 0;
    int findInfoResult = 0;
    std::vector<FFMPEG::Stream> streams;
    bool findInfoWasValid() const;
    bool openFileWasValid() const;
    std::map<std::string, std::string> metadata;
    std::map< std::string, std::string > readMetadata( AVDictionary *metadata );
    FFMPEG::Stream streamFromAV( AVStream *stream );
    Logger logger;
    boost::mutex mutex;
  private:
    class FFMPEGMedia *const q;
};
#endif // FFMPEGMEDIAPRIVATE_H
