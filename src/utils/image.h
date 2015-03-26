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

#ifndef IMAGE_H
#define IMAGE_H

#include "utils/d_ptr.h"
#include <vector>
#include <stdint.h>

typedef std::vector<uint8_t> ImageBlob;

class Image
{
public:
    Image(const std::string &filename);
    Image(const ImageBlob &imageBlob);
    Image(const Image &image);
    ~Image();

    operator ImageBlob() const;
    Image &resize(uint32_t size, uint32_t quality = 75 );
    std::shared_ptr< Image > scaled(uint32_t size, uint32_t quality = 75) const;
private:
    Image();
    D_PTR;
};

#endif // IMAGE_H
