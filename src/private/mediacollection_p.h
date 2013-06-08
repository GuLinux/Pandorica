#ifndef MEDIACOLLECTIONPRIVATE_H
#define MEDIACOLLECTIONPRIVATE_H
#include <boost/filesystem.hpp>
#include <Wt/WSignal>
#include "media.h"

class Settings;
namespace Wt {
class WApplication;
}

class Session;
namespace StreamingPrivate {
class MediaCollectionPrivate {
public:
  MediaCollectionPrivate(Settings *settings, Session *session, Wt::WApplication *app) : settings(settings), session(session), app(app) {}
  void listDirectory(boost::filesystem::path path);
  bool isAllowed(boost::filesystem::path path);
public:
  Wt::WLoadingIndicator *loadingIndicator;
  Settings *settings;
  std::map<std::string,Media> collection;
  Wt::Signal<> scanned;
  Session *session;
  std::list<std::string> allowedPaths;
  Wt::WApplication *app;
  long long userId;
};
}
#endif
