#ifndef MEDIACOLLECTIONPRIVATE_H
#define MEDIACOLLECTIONPRIVATE_H
#include <boost/filesystem.hpp>
#include <Wt/WSignal>
#include "media.h"

class Session;
namespace StreamingPrivate {
class MediaCollectionPrivate {
public:
  MediaCollectionPrivate(std::string basePath, Session *session) : basePath(basePath), session(session) {}
  void listDirectory(boost::filesystem::path path);
  bool isAllowed(boost::filesystem::path path);
public:
  boost::filesystem::path basePath;
  std::map<std::string,Media> collection;
  Wt::Signal<> scanned;
  Session *session;
  std::list<std::string> allowedPaths;
};
}
#endif
