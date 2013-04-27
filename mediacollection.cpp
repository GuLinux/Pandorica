#include "mediacollection.h"
#include "session.h"
#include <Wt/Utils>
#include <Wt/WApplication>

using namespace Wt;
using namespace std;
using namespace boost;
using namespace Wt::Utils;
namespace fs = boost::filesystem;

class MediaCollectionPrivate {
public:
  MediaCollectionPrivate(string basePath, Session *session) : basePath(basePath), session(session) {}
    void listDirectory(filesystem::path path);
    bool isAllowed(filesystem::path path);
public:
  fs::path basePath;
  map<string,Media> collection;
  Signal<> scanned;
  Session *session;
  list< string > allowedPaths;
};

MediaCollection::MediaCollection(string basePath, Session* session, WObject* parent)
  : WObject(parent), d(new MediaCollectionPrivate(basePath, session))
{
}

void MediaCollection::rescan()
{
  Dbo::Transaction t(*d->session);
  d->allowedPaths = d->session->user()->allowedPaths();
  d->collection.clear();
  d->listDirectory(d->basePath);
  d->scanned.emit();
}

bool MediaCollectionPrivate::isAllowed(filesystem::path path)
{
  for(string p: allowedPaths) {
    if(path.string().find(p) != string::npos)
      return true;
  }
  return false;
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
      Media media{path};
      if(media.mimetype() != "UNSUPPORTED" && isAllowed(path)) {
        collection[media.uid()] = media;
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
