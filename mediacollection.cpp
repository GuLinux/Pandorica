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

class MediaPrivate {
public:
  MediaPrivate(fs::path path) : path{path}, uid(hexEncode(md5(path.string()))) {}
  const fs::path path;
  const string uid;
};
Media::Media(filesystem::path path) : d(new MediaPrivate{path}) {}
Media::Media(const Media& other) : Media(other.d->path) {}

Media::~Media()
{
  delete d;
}
string Media::fullPath() const
{
  return d->path.string();
}
string Media::extension() const
{
  return d->path.extension().string();
}
string Media::filename() const
{
  return d->path.filename().string();
}
string Media::mimetype() const
{
  if(supportedMimetypes.count(extension()) >0)
    return supportedMimetypes[extension()];
  return "UNSUPPORTED";
}
filesystem::path Media::path() const
{
  return d->path;
}
string Media::uid() const
{
  return d->uid;
}




class MediaCollectionPrivate {
public:
  MediaCollectionPrivate(string basePath) : basePath(basePath) {}
    void listDirectory(filesystem::path path);
  
public:
  string basePath;
  vector<Media> collection;
};

MediaCollection::MediaCollection(string basePath, WObject* parent)
  : WObject(parent), d(new MediaCollectionPrivate(basePath))
{
  rescan();
}

void MediaCollection::rescan()
{
  d->collection.clear();
  d->listDirectory(fs::path(d->basePath));
  for(Media media: d->collection)
    wApp->log("notice") << "found media: " << media.fullPath();
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
      if(media.mimetype() != "UNSUPPORTED")
        collection.push_back(media);
    }
  }
}



MediaCollection::~MediaCollection()
{
  delete d;
}

