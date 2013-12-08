/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  <copyright holder> <email>
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

#ifndef IMAGE_P_H
#define IMAGE_P_H
#include "utils/image.h"
#ifdef IMAGE_USE_GRAPHICSMAGICK
#include <Magick++/Blob.h>
#else
#include <QImage>
#endif

class Image::Private
{
public:
    Private(Image* q);
#ifdef IMAGE_USE_GRAPHICSMAGICK
    Magick::Blob blob;
#else
    QImage image;
    int quality = 100;
#endif

private:
    class Image* const q;
};
#endif // IMAGE_P_H