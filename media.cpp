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
#include <boost/filesystem.hpp>

using namespace Wt;
using namespace std;
using namespace boost;
using namespace Wt::Utils;
namespace fs = boost::filesystem;


typedef pair<string,string> mimepair;
map<string,string> supportedMimetypes{
  mimepair(".mp4", "video/mp4"),
  mimepair(".m4v", "video/mp4"),
  mimepair(".ogv", "video/ogg"),
  mimepair(".webm", "video/webm"),
  mimepair(".flv", "video/x-flv"),
  mimepair(".ogg", "audio/ogg"),
  mimepair(".mp3", "audio/mpeg")
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

filesystem::path Media::preview(Settings *settings) const
{
  fs::path previewPath = fs::absolute(fs::path(uid() + "/preview.png"), settings->mediaData());
  wApp->log("notice") << "previewPath=" << previewPath;
  return previewPath;
}

list< filesystem::path > Media::subtitles(Settings* settings) const
{
  list<fs::path> subtitles;
  fs::path subtitlesDirectory = fs::absolute(fs::path(uid() + "/subs/"), settings->mediaData());
  if(! (fs::exists(subtitlesDirectory) && fs::is_directory(subtitlesDirectory)))
    return subtitles;
  for(fs::directory_iterator it=fs::directory_iterator(subtitlesDirectory); it!=fs::directory_iterator(); it++) {
    if(it->path().extension() == ".vtt") {
      subtitles.push_back(it->path());
    }
  }
}




#include "media.h"

