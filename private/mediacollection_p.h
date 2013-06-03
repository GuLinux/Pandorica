#ifndef MEDIACOLLECTIONPRIVATE_H
#define MEDIACOLLECTIONPRIVATE_H
#include <boost/filesystem.hpp>
#include <Wt/WSignal>
#include "media.h"

namespace Wt {
class WApplication;
}

class Session;
namespace StreamingPrivate {
class MediaCollectionPrivate {
public:
  MediaCollectionPrivate(std::vector<std::string> mediaDirectories, Session *session, Wt::WApplication *app) : mediaDirectories(mediaDirectories), session(session), app(app) {}
  void listDirectory(boost::filesystem::path path);
  bool isAllowed(boost::filesystem::path path);
public:
  std::vector<std::string> mediaDirectories;
  std::map<std::string,Media> collection;
  Wt::Signal<> scanned;
  Session *session;
  std::list<std::string> allowedPaths;
  Wt::WApplication *app;
  long long userId;
};
}
#endif
