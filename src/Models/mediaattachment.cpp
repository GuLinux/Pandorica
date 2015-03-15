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


#include "Models/models.h"
#include "session.h"

#include <Wt/WApplication>
#include <Wt/WMemoryResource>
#include <Wt/WFileResource>
#include <Wt/WServer>
#include <Wt/Http/Response>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <fstream>
#include <functional>
#include <mutex>

using namespace Wt;
using namespace std;
namespace fs = boost::filesystem;
typedef Wt::Dbo::dbo_default_traits::IdType MediaAttachmentId;

class Cache {
public:
  struct File {
    string name;
    string mimetype;
    fs::path path;
    uint64_t data_size;
    operator bool() const { return !path.empty(); }
    void stream(std::ostream &out) const;
  };
  Cache();
  ~Cache();
  void clean();
  File create(const MediaAttachmentPtr &mediaAttachment) const;
private:
  fs::path cache_directory;
};

Cache::Cache() : cache_directory(fs::temp_directory_path() / (boost::format("pandorica-%s.cache") % std::getenv("USER")).str() )
{
  clean();
  fs::create_directories(cache_directory);
}

Cache::~Cache()
{
  clean();
}

void Cache::clean()
{
  fs::remove_all(cache_directory);
}

Cache::File Cache::create(const MediaAttachmentPtr &mediaAttachment) const
{
  std::cerr << "creating cache file entry: id=" << mediaAttachment.id() << ", name=" << mediaAttachment->name() << ", content type: " << mediaAttachment->mimetype() << std::endl;
  auto file_path = cache_directory / boost::lexical_cast<string>(mediaAttachment.id());
  auto data = mediaAttachment->data();
  ofstream file_out(file_path.string());
  copy(data.begin(), data.end(), ostream_iterator<uint8_t>(file_out));

  return {mediaAttachment->name(), mediaAttachment->mimetype(), file_path, data.size() };
}

void Cache::File::stream(ostream &out) const
{
  ifstream file(path.string());
  copy(istream_iterator<uint8_t>(file), istream_iterator<uint8_t>(), ostream_iterator<uint8_t>(out));
}


class MediaAttachmentResource : public Wt::WFileResource {
public:
    MediaAttachmentResource(const Cache::File &cache_file, WObject* parent = 0);
};

MediaAttachmentResource::MediaAttachmentResource(const Cache::File &cache_file, WObject* parent): WFileResource(cache_file.mimetype, cache_file.path.string(), parent)
{
  suggestFileName(WString::fromUTF8(cache_file.name), Inline);
}

Wt::WLink MediaAttachment::link(Dbo::ptr< MediaAttachment > myPtr, Dbo::Transaction &transaction, WObject* parent, bool useCacheIfAvailable) const
{
  static std::map<MediaAttachmentId, WResource*> resources_map;
  static Cache cache;
  string attachment_path = (boost::format("/media_attachments/%d") % myPtr.id()).str();
  if(resources_map.count(myPtr.id()) == 0) {
    resources_map[myPtr.id()] = new MediaAttachmentResource{cache.create(myPtr)};
    WServer::instance()->addResource(resources_map[myPtr.id()], attachment_path );
  }
  return {attachment_path};
}
