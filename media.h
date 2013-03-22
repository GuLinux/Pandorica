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
#include <Wt/Dbo/ptr>

namespace Wt {
namespace Dbo {
class Transaction;
}
}

class Session;
class MediaAttachment;

class Media {
public:
  enum PreviewSize {PreviewFull, PreviewPlayer, PreviewThumb};
  Media(const boost::filesystem::path &path);
  Media();
  ~Media();
  std::string fullPath() const;
  std::string filename() const;
  std::string extension() const;
  std::string mimetype() const;
  std::string uid() const;
  Wt::Dbo::ptr<MediaAttachment> preview(Session *session, PreviewSize size = PreviewPlayer) const;
  Wt::Dbo::collection<Wt::Dbo::ptr<MediaAttachment>> subtitles(Session *session) const;
  Wt::Dbo::collection<Wt::Dbo::ptr<MediaAttachment>> subtitles(Wt::Dbo::Transaction *transaction) const;
  boost::filesystem::path path() const;
  boost::filesystem::path parentDirectory() const;
  bool valid() const;
private:
  boost::filesystem::path m_path;
  std::string m_uid;
};

#endif // MEDIA_H
