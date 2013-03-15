#include "mediacollection.h"
#include <boost/filesystem.hpp>
#include <Wt/Utils>
#include <Wt/WApplication>

using namespace Wt;
using namespace std;
using namespace boost;
using namespace Wt::Utils;
namespace fs = boost::filesystem;

typedef pair<string,string> mimepair;
map<string,string> supportedMimetypes{
  mimepair(".mp4", "video/mp4"),
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



class MediaCollectionPrivate {
public:
  MediaCollectionPrivate(string basePath) : basePath(basePath) {}
    void listDirectory(filesystem::path path);
  
public:
  fs::path basePath;
  map<string,Media> collection;
  Signal<Media> added;
};

MediaCollection::MediaCollection(string basePath, WObject* parent)
  : WObject(parent), d(new MediaCollectionPrivate(basePath))
{
  rescan();
}

void MediaCollection::rescan()
{
  d->collection.clear();
  d->listDirectory(d->basePath);
  for(pair<string,Media> media: d->collection)
    wApp->log("notice") << "found media with id=" << media.first << ": " << media.second.fullPath();
}

void MediaCollectionPrivate::listDirectory(filesystem::path path)
{
  vector<fs::path> v;
  copy(fs::directory_iterator(path), fs::directory_iterator(), back_inserter(v));
  sort(v.begin(), v.end());
  for(fs::path path: v) {
    if(fs::is_directory(path))
      listDirectory(path);
    else {
      Media media(path);
      if(media.mimetype() != "UNSUPPORTED") {
        collection[media.uid()] = media;
        added.emit(media);
      }
    }
  }
}

map< string, Media > MediaCollection::collection() const
{
  return d->collection;
}

Media MediaCollection::media(string uid) const
{
  return d->collection[uid];
}

Wt::Signal< Media >& MediaCollection::added()
{
  return d->added;
}



MediaCollection::~MediaCollection()
{
  delete d;
}

filesystem::path MediaCollection::rootPath() const
{
  return d->basePath;
}
