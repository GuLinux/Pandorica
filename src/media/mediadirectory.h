/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Marco Gulino <email>
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

#ifndef MEDIADIRECTORY_H
#define MEDIADIRECTORY_H
#include "utils/d_ptr.h"
#include <boost/filesystem.hpp>
#include <ostream>
class Media;
class MediaDirectory : public std::enable_shared_from_this<MediaDirectory>
{
  public:
    MediaDirectory( const boost::filesystem::path &path, const std::shared_ptr<MediaDirectory> &parent = std::shared_ptr<MediaDirectory>() );
    ~MediaDirectory();
    virtual std::vector<std::shared_ptr<MediaDirectory>> subDirectories() const;
    virtual std::vector<Media> medias() const;
    virtual std::vector<Media> allMedias() const;
    std::string label() const;
    void add( const Media &media );
    bool operator ==(const MediaDirectory &other) const;
    boost::filesystem::path path() const;
    friend std::ostream & operator<<( std::ostream &os, const MediaDirectory &md );
    void setLabel(const std::string &label);
    std::shared_ptr<MediaDirectory> parent();
    std::shared_ptr<MediaDirectory> ptr();
    bool contains(const Media &media) const;
    bool recursiveContains(const Media &media) const;
    std::string relativePath();
  private:
    D_PTR;
};

#endif // MEDIADIRECTORY_H
