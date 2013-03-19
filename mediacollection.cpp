#include "mediacollection.h"
#include <Wt/Utils>
#include <Wt/WApplication>

using namespace Wt;
using namespace std;
using namespace boost;
using namespace Wt::Utils;
namespace fs = boost::filesystem;

class MediaCollectionPrivate {
public:
  MediaCollectionPrivate(string basePath) : basePath(basePath) {}
    void listDirectory(filesystem::path path);
  
public:
  fs::path basePath;
  map<string,Media> collection;
  Signal<Media> added;
  Signal<> scanned;
};

MediaCollection::MediaCollection(string basePath, WObject* parent)
  : WObject(parent), d(new MediaCollectionPrivate(basePath))
{
}

void MediaCollection::rescan()
{
  d->collection.clear();
  d->listDirectory(d->basePath);
  d->scanned.emit();
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

Signal<>& MediaCollection::scanned()
{
  return d->scanned;
}




MediaCollection::~MediaCollection()
{
  delete d;
}

filesystem::path MediaCollection::rootPath() const
{
  return d->basePath;
}
