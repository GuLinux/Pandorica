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

#include "image.h"
#include "private/image_p.h"
#include "utils/d_ptr_implementation.h"

#ifdef IMAGE_USE_GRAPHICSMAGICK
#include <Magick++/Image.h>
#include <Magick++/Geometry.h>

Image::Image(const std::string& filename)
    : d(this)
{
    Magick::Image fullImage( filename);
    fullImage.quality( 100 );
    fullImage.write( &d->blob );
}
Image::Image(const ImageBlob& imageBlob)
    : d(this)
{
  d->blob = Magick::Blob(imageBlob.data(), imageBlob.size());
}

Image& Image::resize(uint32_t size, uint32_t quality)
{
  Magick::Image image {d->blob};
  image.sample( {size, size} );
  image.quality( quality );
  image.write( &d->blob );
  return *this;
}

Image::operator ImageBlob() const
{
  const uint8_t *outData = static_cast<const uint8_t*>(d->blob.data());
  return {outData, outData + d->blob.length()};
}

#else
#include <QBuffer>

Image::Image(const std::string& filename)
    : d(this)
{
  d->image.load(QString::fromStdString(filename));
}
Image::Image(const ImageBlob& imageBlob)
    : d(this)
{
  d->image.loadFromData(imageBlob.data(), imageBlob.size());
}

Image& Image::resize(uint32_t size, uint32_t quality)
{
  d->image = d->image.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  d->quality = quality;
  return *this;
}

Image::operator ImageBlob() const
{
  QByteArray ba;
  QBuffer buffer(&ba);
  buffer.open(QIODevice::WriteOnly);
  d->image.save(&buffer, "PNG", d->quality);
  buffer.close();
  return {ba.begin(), ba.end()};
}
#endif



Image::Private::Private(Image* q) : q(q)
{
}


Image::~Image()
{
}
