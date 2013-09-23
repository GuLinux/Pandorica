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


#ifndef MEDIA_ATTACHMENT_H
#define MEDIA_ATTACHMENT_H

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/Types>
#include <Wt/Dbo/ptr>
#include <Wt/WLink>

namespace Wt {
class WObject;
class WResource;
}


class MediaAttachment {
public:
  template<class Action>
  void persist(Action& a) {
    Wt::Dbo::field(a, _mediaId, "media_id", 32);
    Wt::Dbo::field(a, _type, "type");
    Wt::Dbo::field(a, _name, "name");
    Wt::Dbo::field(a, _value, "value");
    Wt::Dbo::field(a, _mimetype, "mimetype");
    Wt::Dbo::field(a, _data, "data");
  }
  MediaAttachment() = default;
  MediaAttachment(const std::string &type, const std::string &name, const std::string &value, const std::string &mediaId, const std::string &mimetype, const std::vector<unsigned char> &data)
    : _type(type), _name(name), _value(value), _mediaId(mediaId), _mimetype(mimetype), _data(data)
  {}
  inline std::string type() const { return _type; }
  inline std::string name() const { return _name; }
  inline std::string value() const { return _value; }
  inline std::string mediaId() const { return _mediaId; }
  inline std::string mimetype() const { return _mimetype; }
  inline unsigned long size() const { return _data.size(); }
  inline std::vector<unsigned char> data() const { return _data; }
  
  Wt::WLink link(Wt::Dbo::ptr< MediaAttachment > myPtr, Wt::Dbo::Transaction &transaction, Wt::WObject* parent = 0, bool useCacheIfAvailable = true) const;
private:
  std::string _type;
  std::string _name;
  std::string _value;
  std::string _mediaId;
  std::string _mimetype;
  std::vector<unsigned char> _data;
};


typedef Wt::Dbo::ptr<MediaAttachment> MediaAttachmentPtr;
#endif