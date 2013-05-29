#ifndef MEDIACOLLECTION_H
#define MEDIACOLLECTION_H

#include <Wt/WObject>
#include <Wt/WSignal>
#include <boost/filesystem.hpp>
#include "media.h"

namespace Wt {
namespace Dbo {
class Transaction;
}
}

class Session;
namespace StreamingPrivate {
  class MediaCollectionPrivate;
}
class MediaCollection : public Wt::WObject
{
public:
  MediaCollection(std::string basePath, Session *session, Wt::WApplication* parent = 0);
    virtual ~MediaCollection();
    void rescan(Wt::Dbo::Transaction& transaction);
    std::map<std::string,Media> collection() const;
    boost::filesystem::path rootPath() const;
    Media media(std::string uid) const;
    Wt::Signal<> &scanned();
private:
  StreamingPrivate::MediaCollectionPrivate *const d;
};

#endif // MEDIACOLLECTION_H
