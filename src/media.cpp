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



#include "media.h"
#include <Wt/Utils>
#include <map>
#include "settings.h"
#include "Wt/WApplication"
#include "session.h"
#include "settings.h"
#include <boost/filesystem.hpp>
#include "Models/models.h"

using namespace Wt;
using namespace std;
using namespace boost;
using namespace Wt::Utils;
namespace fs = boost::filesystem;


map<string,string> supportedMimetypes{
  {".mp4", "video/mp4"}, {".m4v", "video/mp4"}, {".ogv", "video/ogg"}, {".webm", "video/webm"}, {".flv", "video/x-flv"},
  {".ogg", "audio/ogg"}, {".mp3", "audio/mpeg"}
};

Media::Media(const filesystem::path& path) : m_path{path}, m_uid{hexEncode(md5(path.string()))} {
}

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

WString Media::title(Dbo::Transaction& transaction) const
{
  MediaPropertiesPtr properties = transaction.session().find<MediaProperties>().where("media_id = ?").bind(uid());
  if(!properties || properties->title().empty())
    return WString::fromUTF8(filename());
  return WString::fromUTF8(properties->title());
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
  return !uid().empty() && !path().empty() && ! path().string().empty() && mimetype() != "UNSUPPORTED";
}

Media Media::invalid()
{
  return {};
}


Dbo::ptr< MediaAttachment > Media::preview(Dbo::Transaction& transaction, Media::PreviewSize size ) const
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
  return transaction.session().find<MediaAttachment>().where("media_id = ? AND type = 'preview' AND name = ?")
    .bind(uid()).bind(previewSize);
}


Dbo::collection<MediaAttachmentPtr> Media::subtitles(Dbo::Transaction& transaction) const
{
  return transaction.session().find<MediaAttachment>().where("media_id = ? AND type = 'subtitles'").bind(uid()).resultList();
}

MediaPropertiesPtr Media::properties(Dbo::Transaction& transaction) const
{
  return transaction.session().find<MediaProperties>().where("media_id = ?").bind(uid()).resultValue();
}




#include "media.h"

