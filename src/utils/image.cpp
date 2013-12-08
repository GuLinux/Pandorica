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
#include <Magick++/Image.h>
#include <Magick++/Geometry.h>


Image::Private::Private(Image* q) : q(q)
{
}

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


Image::~Image()
{
}

Image::operator ImageBlob() const
{
  const uint8_t *outData = static_cast<const uint8_t*>(d->blob.data());
  return {outData, outData + d->blob.length()};
}
