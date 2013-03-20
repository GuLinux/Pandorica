/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2013  Marco Gulino <email>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef MEDIA_H
#define MEDIA_H
#include <boost/filesystem.hpp>
class Settings;

class MediaSubtitle {
public:
  MediaSubtitle(const boost::filesystem::path path);
  boost::filesystem::path path() const;
  std::string label() const;
  std::string language() const;
private:
  boost::filesystem::path _path;
  std::string _label;
  std::string _language;
};
class Media {
public:
  Media(const boost::filesystem::path &path);
  Media();
  ~Media();
  std::string fullPath() const;
  std::string filename() const;
  std::string extension() const;
  std::string mimetype() const;
  std::string uid() const;
  boost::filesystem::path preview(Settings *settings) const;
  std::list<MediaSubtitle> subtitles(Settings *settings) const;
  boost::filesystem::path path() const;
  boost::filesystem::path parentDirectory() const;
  bool valid() const;
private:
  boost::filesystem::path m_path;
  std::string m_uid;
};

#endif // MEDIA_H
