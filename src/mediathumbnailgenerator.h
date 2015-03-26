/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MEDIATHUMBNAILGENERATOR_H
#define MEDIATHUMBNAILGENERATOR_H
#include "media/media.h"
#include "utils/image.h"

extern "C" {
#include <libavcodec/avcodec.h>    // required headers
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

class FFMPEGMedia;
class MediaThumbnailGenerator
{
public:
    MediaThumbnailGenerator(FFMPEGMedia* media);
    ~MediaThumbnailGenerator();
    std::shared_ptr<Image> image(int quality, AVFormatContext* pAvContext) const;
private:
  const FFMPEGMedia *media;
};

#endif // MEDIATHUMBNAILGENERATOR_H
