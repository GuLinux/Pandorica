#ifndef MEDIACOLLECTION_H
#define MEDIACOLLECTION_H

#include <Wt/WObject>
#include <Wt/WSignal>
#include <boost/filesystem.hpp>
#include "media.h"

class Session;
class MediaCollectionPrivate;
class MediaCollection : public Wt::WObject
{
public:
  MediaCollection(std::string basePath, Session *session, Wt::WObject* parent = 0);
    virtual ~MediaCollection();
    void rescan();
    std::map<std::string,Media> collection() const;
    boost::filesystem::path rootPath() const;
    Media media(std::string uid) const;
    Wt::Signal<Media> &added();
    Wt::Signal<> &scanned();
private:
  MediaCollectionPrivate *const d;
};

#endif // MEDIACOLLECTION_H
