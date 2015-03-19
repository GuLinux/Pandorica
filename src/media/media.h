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




#ifndef MEDIA_H
#define MEDIA_H
#include <boost/filesystem.hpp>
#include <Wt/Dbo/ptr>
#include <Wt/WString>
#include <ostream>
#include <boost/date_time.hpp>
#include <mutex>

class Image;
class MediaProperties;
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
  Wt::WString title(Wt::Dbo::Transaction &transaction) const;
  std::string fullPath() const;
  std::string filename() const;
  std::string extension() const;
  std::string mimetype() const;
  std::string uid() const;
  Wt::Dbo::ptr<MediaAttachment> preview(Wt::Dbo::Transaction &transaction, PreviewSize size = PreviewPlayer) const;
  Wt::Dbo::collection<Wt::Dbo::ptr<MediaAttachment>> subtitles(Wt::Dbo::Transaction& transaction) const;
  Wt::Dbo::ptr<MediaProperties> properties(Wt::Dbo::Transaction &transaction) const;
  void setImage(const Image& image, Wt::Dbo::Transaction& transaction) const;
  boost::filesystem::path path() const;
  boost::filesystem::path parentDirectory() const;
  bool valid() const;
  static Media invalid();
  bool operator ==(const Media &other) const;
  friend std::ostream & operator<<( std::ostream &os, const Media &m );
  Wt::WDateTime creationTime(Wt::Dbo::Transaction &transaction) const;
  boost::posix_time::ptime posixCreationTime(Wt::Dbo::Transaction &transaction) const;
  typedef std::shared_ptr<std::unique_lock<std::mutex>> Lock;
  Lock lock() const;
private:
  boost::filesystem::path m_path;
  std::string m_uid;
  std::shared_ptr<std::mutex> lock_mutex;
};

#endif // MEDIA_H
