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

#include "media.h"
#include <Wt/Utils>
#include <map>
#include "settings.h"
#include "Wt/WApplication"
#include "mediaattachment.h"
#include "session.h"
#include "group.h"
#include "sessioninfo.h"
#include "user.h"
#include "sessiondetails.h"
#include "comment.h"
#include "settings.h"
#include <boost/filesystem.hpp>

using namespace Wt;
using namespace std;
using namespace boost;
using namespace Wt::Utils;
namespace fs = boost::filesystem;


map<string,string> supportedMimetypes{
  {".mp4", "video/mp4"}, {".m4v", "video/mp4"}, {".ogv", "video/ogg"}, {".webm", "video/webm"}, {".flv", "video/x-flv"},
  {".ogg", "audio/ogg"}, {".mp3", "audio/mpeg"}
};

Media::Media(const filesystem::path& path) : m_path{path}, m_uid{hexEncode(md5(path.string()))} {}

Media::~Media()
{
}
string Media::fullPath() const
{
  return m_path.string();
}
string Media::extension() const
{
  return m_path.extension().string();
}
string Media::filename() const
{
  return m_path.filename().string();
}

WString Media::title(Session* session) const
{
  Dbo::Transaction t(*session);
  MediaPropertiesPtr properties = session->find<MediaProperties>().where("media_id = ?").bind(uid());
  if(!properties || properties->title().empty())
    return filename();
  return properties->title();
}

string Media::mimetype() const
{
  if(supportedMimetypes.count(extension()) >0)
    return supportedMimetypes[extension()];
  return "UNSUPPORTED";
}
filesystem::path Media::path() const
{
  return m_path;
}
filesystem::path Media::parentDirectory() const
{
  return m_path.parent_path();
}

string Media::uid() const
{
  return m_uid;
}

Media::Media() {}

bool Media::valid() const
{
  return !uid().empty() && !path().empty();
}

Dbo::ptr< MediaAttachment > Media::preview(Session* session, Media::PreviewSize size ) const
{
  string previewSize;
  switch(size) {
    case Media::PreviewFull:
      previewSize="full"; break;
    case Media::PreviewPlayer:
      previewSize="player"; break;
    case Media::PreviewThumb:
      previewSize="thumbnail"; break;
  }
  Dbo::Transaction t(*session);
  return session->find<MediaAttachment>().where("media_id = ? AND type = 'preview' AND name = ?")
    .bind(uid()).bind(previewSize);
}

Dbo::collection<MediaAttachmentPtr> Media::subtitles(Session* session) const
{
  Dbo::Transaction t(*session);
  return subtitles(&t);
}

Dbo::collection< Dbo::ptr< MediaAttachment > > Media::subtitles(Dbo::Transaction* transaction) const
{
  Dbo::collection<MediaAttachmentPtr> subs = transaction->session().find<MediaAttachment>().where("media_id = ? AND type = 'subtitles'")
    .bind(uid());
  return subs;
}



#include "media.h"

