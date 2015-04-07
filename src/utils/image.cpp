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
#warning Using GraphicsMagic
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

std::shared_ptr<Image> Image::scaled(uint32_t size, uint32_t quality) const
{
  auto image = std::shared_ptr<Image>(new Image);
  Magick::Image scaled_image {d->blob};
  scaled_image.sample( {size, size} );
  scaled_image.quality( quality );
  scaled_image.write( &image->d->blob );
  return image;
}

Image::Image(const Image& image) : d(this)
{
  d->blob = image.d->blob;
}


#else
#warning Using Qt Image Backend
#include <QBuffer>
#include <Wt/Utils>

Image::Image(const std::string& filename)
    : d(this)
{
  d->image.load(QString::fromStdString(filename));
  d->contentType = Wt::Utils::guessImageMimeType(filename);
}
Image::Image(const ImageBlob& imageBlob)
    : d(this)
{
  d->image.loadFromData(imageBlob.data(), imageBlob.size());
  d->contentType = Wt::Utils::guessImageMimeTypeData(imageBlob);
}

Image& Image::resize(uint32_t size, uint32_t quality)
{
  d->image = d->image.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  d->quality = quality;
  return *this;
}
#include <QDebug>
Image::operator ImageBlob() const
{
  QByteArray ba;
  QBuffer buffer(&ba);
  buffer.open(QIODevice::WriteOnly);
  QString format = QString::fromStdString(d->contentType).split('/')[1].toUpper();
  qDebug() << "content type: " << QString::fromStdString(d->contentType) << ", format: " << format;
  d->image.save(&buffer, format.toLocal8Bit().constData(), d->quality);
  buffer.close();
  return {ba.begin(), ba.end()};
}

std::shared_ptr<Image> Image::scaled(uint32_t size, uint32_t quality) const
{
  auto image = std::shared_ptr<Image>(new Image);
  image->d->image = d->image.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  image->d->quality = quality;
  image->d->contentType = d->contentType;
  return image;
}

Image::Image(const Image& image) : d(this)
{
  d->image = image.d->image;
  d->quality = image.d->quality;
  d->contentType = image.d->contentType;
}


#endif

Image::Image() : d(this)
{
}


Image::Private::Private(Image* q) : q(q)
{
}


Image::~Image()
{
}

ImageBlob Image::blob() const
{
  return *this;
}

