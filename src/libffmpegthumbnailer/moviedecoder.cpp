//    Copyright (C) 2010 Dirk Vanden Boer <dirk.vdb@gmail.com>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "moviedecoder.h"

#include "videoframe.h"

#include <stdexcept>
#include <algorithm>
#include <cassert>
#include <cstring>

extern "C" {
#ifdef LATEST_GREATEST_FFMPEG	
#include <libavutil/opt.h>
#endif
#include <libswscale/swscale.h>
}

#include "ffmpegmedia.h"
#include <threadpool.h>

using namespace std;

namespace ffmpegthumbnailer
{

MovieDecoder::MovieDecoder(const string& filename, AVFormatContext* pavContext)
: m_VideoStream(-1)
, m_pFormatContext(pavContext)
, m_pVideoCodecContext(NULL)
, m_pVideoCodec(NULL)
, m_pVideoStream(NULL)
, m_pFrame(NULL)
, m_pFrameBuffer(NULL)
, m_pPacket(NULL)
, m_FormatContextWasGiven(pavContext != NULL)
, m_AllowSeek(true)
{
    initialize(filename);
}

MovieDecoder::~MovieDecoder()
{
    destroy();
}

void MovieDecoder::initialize(const string& filename)
{
    av_register_all();
    avcodec_register_all();

    auto lock = ThreadPool::lock("ffmpeg_avcodec_open");
    string inputFile = filename == "-" ? "pipe:" : filename;
    m_AllowSeek = (filename != "-") && (filename.find("rtsp://") != 0);
#if LIBAVCODEC_VERSION_MAJOR < 53
    if ((!m_FormatContextWasGiven) && av_open_input_file(&m_pFormatContext, inputFile.c_str(), NULL, 0, NULL) != 0)
#else
        if ((!m_FormatContextWasGiven) && avformat_open_input(&m_pFormatContext, inputFile.c_str(), NULL, NULL) != 0)
#endif
    {
                destroy();
        throw logic_error(string("Could not open input file: ") + filename);
    }
#if LIBAVCODEC_VERSION_MAJOR < 53
    if (av_find_stream_info(m_pFormatContext) < 0)
#else
	if (avformat_find_stream_info(m_pFormatContext, NULL) < 0)
#endif
    {
		destroy();
        throw logic_error(string("Could not find stream information"));
    }

    initializeVideo();
    m_pFrame = avcodec_alloc_frame();
}

void MovieDecoder::destroy()
{
    if (m_pVideoCodecContext)
    {
      auto lock = ThreadPool::lock("ffmpeg_avcodec_open");
      avcodec_close(m_pVideoCodecContext);
      m_pVideoCodecContext = NULL;
    }

    if ((!m_FormatContextWasGiven) && m_pFormatContext)
    {
#if LIBAVCODEC_VERSION_MAJOR < 53
        av_close_input_file(m_pFormatContext);
        m_pFormatContext = NULL;
#else
        avformat_close_input(&m_pFormatContext);
#endif
    }

    if (m_pPacket)
    {
        av_free_packet(m_pPacket);
        delete m_pPacket;
        m_pPacket = NULL;
    }

    if (m_pFrame)
    {
        av_free(m_pFrame);
        m_pFrame = NULL;
    }

    if (m_pFrameBuffer)
    {
        av_free(m_pFrameBuffer);
        m_pFrameBuffer = NULL;
    }

    m_VideoStream = -1;
}

string MovieDecoder::getCodec()
{
    if (m_pVideoCodec)
    {
        return m_pVideoCodec->name;
    }

    return "";
}

void MovieDecoder::initializeVideo()
{
    for (unsigned int i = 0; i < m_pFormatContext->nb_streams; ++i)
    {
#if LIBAVCODEC_VERSION_MAJOR < 53
        if (m_pFormatContext->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO)
#else
        if (m_pFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
#endif
        {
            m_pVideoStream = m_pFormatContext->streams[i];
            m_VideoStream = i;
            break;
        }
    }

    if (m_VideoStream == -1)
    {
        throw logic_error("Could not find video stream");
    }

    m_pVideoCodecContext = m_pFormatContext->streams[m_VideoStream]->codec;
    m_pVideoCodec = avcodec_find_decoder(m_pVideoCodecContext->codec_id);

    if (m_pVideoCodec == NULL)
    {
        // set to NULL, otherwise avcodec_close(m_pVideoCodecContext) crashes
        m_pVideoCodecContext = NULL;
        throw logic_error("Video Codec not found");
    }

    m_pVideoCodecContext->workaround_bugs = 1;

    {
//       auto lock = ThreadPool::lock("ffmpeg_avcodec_open");
#if LIBAVCODEC_VERSION_MAJOR < 53
      if (avcodec_open(m_pVideoCodecContext, m_pVideoCodec) < 0)
#else
	  if (avcodec_open2(m_pVideoCodecContext, m_pVideoCodec, NULL) < 0)
#endif
      {
	  throw logic_error("Could not open video codec");
      }
    }
}

int MovieDecoder::getWidth()
{
    if (m_pVideoCodecContext)
    {
        return m_pVideoCodecContext->width;
    }

    return -1;
}

int MovieDecoder::getHeight()
{
    if (m_pVideoCodecContext)
    {
        return m_pVideoCodecContext->height;
    }

    return -1;
}

int MovieDecoder::getDuration()
{
    if (m_pFormatContext)
    {
        return static_cast<int>(m_pFormatContext->duration / AV_TIME_BASE);
    }

    return 0;
}

void MovieDecoder::seek(int timeInSeconds)
{
    auto lock = ThreadPool::lock("ffmpeg_avcodec_open");
    if (!m_AllowSeek)
    {
        return;
    }
    
    int64_t timestamp = AV_TIME_BASE * static_cast<int64_t>(timeInSeconds);

    if (timestamp < 0)
    {
        timestamp = 0;
    }

    int ret = av_seek_frame(m_pFormatContext, -1, timestamp, 0);
    if (ret >= 0)
    {
        avcodec_flush_buffers(m_pFormatContext->streams[m_VideoStream]->codec);
    }
    else
    {
        throw logic_error("Seeking in video failed");
    }

    int keyFrameAttempts = 0;
    bool gotFrame = 0;
    
    do
    {
        int count = 0;
        gotFrame = 0;

        while (!gotFrame && count < 20)
        {
            getVideoPacket();
            try
            {
                gotFrame = decodeVideoPacket();
            }
            catch(logic_error&) {}
            ++count;
        }
        
        ++keyFrameAttempts;
    } while ((!gotFrame || !m_pFrame->key_frame) && keyFrameAttempts < 200);
    
    if (gotFrame == 0)
    {
        throw logic_error("Seeking in video failed");
    }
}


void MovieDecoder::decodeVideoFrame()
{
    bool frameFinished = false;

    while(!frameFinished && getVideoPacket())
    {
        frameFinished = decodeVideoPacket();
    }

    if (!frameFinished)
    {
        throw logic_error("decodeVideoFrame() failed: frame not finished");
    }
}

bool MovieDecoder::decodeVideoPacket()
{
    if (m_pPacket->stream_index != m_VideoStream)
    {
        return false;
    }

    avcodec_get_frame_defaults(m_pFrame);
    
    int frameFinished;
    
    int bytesDecoded = avcodec_decode_video2(m_pVideoCodecContext, m_pFrame, &frameFinished, m_pPacket);
    if (bytesDecoded < 0)
    {
        throw logic_error("Failed to decode video frame: bytesDecoded < 0");
    }

    return frameFinished > 0;
}

bool MovieDecoder::getVideoPacket()
{
    bool framesAvailable = true;
    bool frameDecoded = false;

    int attempts = 0;
    
    if (m_pPacket)
    {
        av_free_packet(m_pPacket);
        delete m_pPacket;
    }

    m_pPacket = new AVPacket();

    while (framesAvailable && !frameDecoded && (attempts++ < 1000))
    {
        framesAvailable = av_read_frame(m_pFormatContext, m_pPacket) >= 0;
        if (framesAvailable)
        {
            frameDecoded = m_pPacket->stream_index == m_VideoStream;
            if (!frameDecoded)
            {
                av_free_packet(m_pPacket);
            }
        }
    }
    
    return frameDecoded;
}

void MovieDecoder::getScaledVideoFrame(int scaledSize, bool maintainAspectRatio, VideoFrame& videoFrame)
{
    if (m_pFrame->interlaced_frame)
    {
        avpicture_deinterlace((AVPicture*) m_pFrame, (AVPicture*) m_pFrame, m_pVideoCodecContext->pix_fmt,
                              m_pVideoCodecContext->width, m_pVideoCodecContext->height);
    }

    int scaledWidth, scaledHeight;
    convertAndScaleFrame(PIX_FMT_RGB24, scaledSize, maintainAspectRatio, scaledWidth, scaledHeight);

    videoFrame.width = scaledWidth;
    videoFrame.height = scaledHeight;
    videoFrame.lineSize = m_pFrame->linesize[0];

    videoFrame.frameData.clear();
    videoFrame.frameData.resize(videoFrame.lineSize * videoFrame.height);
    memcpy((&(videoFrame.frameData.front())), m_pFrame->data[0], videoFrame.lineSize * videoFrame.height);
}

void MovieDecoder::convertAndScaleFrame(PixelFormat format, int scaledSize, bool maintainAspectRatio, int& scaledWidth, int& scaledHeight)
{
    calculateDimensions(scaledSize, maintainAspectRatio, scaledWidth, scaledHeight);

#ifdef LATEST_GREATEST_FFMPEG
	// Enable this when it hits the released ffmpeg version
    SwsContext* scaleContext = sws_alloc_context();
    if (scaleContext == NULL)
    {
		throw std::logic_error("Failed to allocate scale context");
	}
	
	av_set_int(scaleContext, "srcw", m_pVideoCodecContext->width);
    av_set_int(scaleContext, "srch", m_pVideoCodecContext->height);
    av_set_int(scaleContext, "src_format", m_pVideoCodecContext->pix_fmt);
    av_set_int(scaleContext, "dstw", scaledWidth);
    av_set_int(scaleContext, "dsth", scaledHeight);
    av_set_int(scaleContext, "dst_format", format);
	av_set_int(scaleContext, "sws_flags", SWS_BICUBIC);
	
	const int* coeff = sws_getCoefficients(SWS_CS_DEFAULT);
    if (sws_setColorspaceDetails(scaleContext, coeff, m_pVideoCodecContext->pix_fmt, coeff, format, 0, 1<<16, 1<<16) < 0)
    {
		sws_freeContext(scaleContext);
		throw std::logic_error("Failed to set colorspace details");
	}

	if (sws_init_context(scaleContext, NULL, NULL) < 0)
	{
		sws_freeContext(scaleContext);
		throw std::logic_error("Failed to initialise scale context");
	}
#endif
    
    SwsContext* scaleContext = sws_getContext(m_pVideoCodecContext->width, m_pVideoCodecContext->height,
                                              m_pVideoCodecContext->pix_fmt, scaledWidth, scaledHeight,
                                              format, SWS_BICUBIC, NULL, NULL, NULL);

    if (NULL == scaleContext)
    {
        throw logic_error("Failed to create resize context");
    }

    AVFrame* convertedFrame = NULL;
    uint8_t* convertedFrameBuffer = NULL;

    createAVFrame(&convertedFrame, &convertedFrameBuffer, scaledWidth, scaledHeight, format);
    
    sws_scale(scaleContext, m_pFrame->data, m_pFrame->linesize, 0, m_pVideoCodecContext->height,
              convertedFrame->data, convertedFrame->linesize);
    sws_freeContext(scaleContext);

    av_free(m_pFrame);
    av_free(m_pFrameBuffer);
    
    m_pFrame        = convertedFrame;
    m_pFrameBuffer  = convertedFrameBuffer;
}

void MovieDecoder::calculateDimensions(int squareSize, bool maintainAspectRatio, int& destWidth, int& destHeight)
{
    if (squareSize == 0)
    {
        // use original video size
        squareSize = max(m_pVideoCodecContext->width, m_pVideoCodecContext->height);
    }
    
    if (!maintainAspectRatio)
    {
        destWidth = squareSize;
        destHeight = squareSize;
    }
    else
    {
        int srcWidth            = m_pVideoCodecContext->width;
        int srcHeight           = m_pVideoCodecContext->height;
        int ascpectNominator    = m_pVideoCodecContext->sample_aspect_ratio.num;
        int ascpectDenominator  = m_pVideoCodecContext->sample_aspect_ratio.den;
        
        if (ascpectNominator != 0 && ascpectDenominator != 0)
        {
            srcWidth = srcWidth * ascpectNominator / ascpectDenominator;
        }
        
        if (srcWidth > srcHeight)
        {
            destWidth  = squareSize;
            destHeight = static_cast<int>(static_cast<float>(squareSize) / srcWidth * srcHeight);
        }
        else
        {
            destWidth  = static_cast<int>(static_cast<float>(squareSize) / srcHeight * srcWidth);
            destHeight = squareSize;
        }
    }
}

void MovieDecoder::createAVFrame(AVFrame** pAvFrame, uint8_t** pFrameBuffer, int width, int height, PixelFormat format)
{
    *pAvFrame = avcodec_alloc_frame();

    int numBytes = avpicture_get_size(format, width, height);
    *pFrameBuffer = reinterpret_cast<uint8_t*>(av_malloc(numBytes));
    avpicture_fill((AVPicture*) *pAvFrame, *pFrameBuffer, format, width, height);
}

}

