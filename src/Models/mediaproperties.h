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


#ifndef MEDIAPROPERTIES_H
#define MEDIAPROPERTIES_H

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/Types>
#include <Wt/Dbo/ptr>
#include <Wt/WDateTime>

class MediaProperties {
public:
  template<class Action>
  void persist(Action& a) {
    Wt::Dbo::id(a, _mediaId, "media_id", 32);
    Wt::Dbo::field(a, _title, "title");
    Wt::Dbo::field(a, _filename, "filename");
    Wt::Dbo::field(a, _duration, "duration");
    Wt::Dbo::field(a, _size, "size");
    Wt::Dbo::field(a, _width, "width");
    Wt::Dbo::field(a, _height, "height");
    Wt::Dbo::field(a, _creationTime, "creation_time");
  }
  std::string mediaId() const { return _mediaId; }
  std::string title() const { return _title; }
  std::string filename() const { return _filename; }
  int64_t duration() const { return _duration; }
  int64_t size() const { return _size; }
  int width() const { return _width; }
  int height() const { return _height; }
  Wt::WDateTime creationTime() const { return Wt::WDateTime::fromPosixTime(_creationTime); }
  boost::posix_time::ptime posixCreationTime() const { return _creationTime; }
  void setTitle(const std::string &title) { _title = title; }
  void setFileName(const std::string &fileName) { _filename = fileName; }
  MediaProperties() = default;
  // TODO: int64_t => uint64_t
  MediaProperties(std::string mediaId, std::string title, std::string filename, int64_t duration, int64_t size, int width, int height)
  : _mediaId(mediaId), _title(title), _filename(filename), _duration(duration), _size(size), _width(width), _height(height), _creationTime(Wt::WDateTime::currentDateTime().toPosixTime()) {}
private:
  std::string _mediaId;
  std::string _title;
  std::string _filename;
  int64_t _duration;
  int64_t _size;
  int _width;
  int _height;
  boost::posix_time::ptime _creationTime;
};



namespace Wt {
  namespace Dbo {
    template<>
    struct dbo_traits<MediaProperties> {
      typedef std::string IdType;
      static IdType invalidId() { return {}; }
      static const char *surrogateIdField() { return 0; }
      static const char *versionField() { return 0; }
    };
  }
}
typedef Wt::Dbo::ptr<MediaProperties> MediaPropertiesPtr;

#endif