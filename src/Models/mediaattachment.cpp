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
#include <session.h>

#include <Wt/WApplication>
#include <Wt/WMemoryResource>
#include <Wt/WFileResource>
#include <Wt/WServer>
#include <Wt/Http/Response>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <fstream>
#include <functional>

using namespace Wt;
using namespace std;
namespace fs = boost::filesystem;

class MediaAttachmentResource : public Wt::WStreamResource {
public:
    MediaAttachmentResource(Wt::Dbo::dbo_default_traits::IdType id, WObject* parent = 0);
    virtual void handleRequest(const Http::Request& request, Http::Response& response);
private:
  Wt::Dbo::dbo_default_traits::IdType id;
};

void MediaAttachmentResource::handleRequest(const Http::Request& request, Http::Response& response)
{
  static Session session;
  Dbo::Transaction t(session);
  MediaAttachmentPtr attachment = session.find<MediaAttachment>().where("id = ?").bind(id);
  if(! attachment) {
    response.out() << "Not Found";
    response.setStatus(404);
    return;
  }
  setMimeType(attachment->mimetype());
  std::ostream_iterator<uint8_t> out(response.out());
  auto data = attachment->data();
  std::copy(data.begin(), data.end(), out);
}

MediaAttachmentResource::MediaAttachmentResource(Dbo::dbo_default_traits::IdType id, WObject* parent): WStreamResource(parent), id(id)
{
}


Wt::WLink MediaAttachment::link(Dbo::ptr< MediaAttachment > myPtr, Dbo::Transaction &transaction, WObject* parent, bool useCacheIfAvailable) const
{
  static std::map<Wt::Dbo::dbo_default_traits::IdType, WResource*> resources_map;
  string attachment_path = (boost::format("/media_attachments/%d") % myPtr.id()).str();
  if(resources_map.count(myPtr.id()) == 0) {
    resources_map[myPtr.id()] = new MediaAttachmentResource{myPtr.id()};
    WServer::instance()->addResource(resources_map[myPtr.id()], attachment_path );
  }
  return {attachment_path};
}
