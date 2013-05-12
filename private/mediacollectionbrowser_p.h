#ifndef MEDIACOLLECTIONBROWSERPRIVATE_H
#define MEDIACOLLECTIONBROWSERPRIVATE_H

#include <boost/filesystem.hpp>
#include <Wt/WSignal>
#include <media.h>
#include "Wt-Commons/wt_helpers.h"

class MediaCollectionBrowser;
namespace Wt {
class WContainerWidget;
}

class Session;
class Settings;
class MediaCollection;
namespace StreamingPrivate {
typedef std::pair<std::string,Media> MediaEntry;
typedef std::function<std::string(Wt::WObject*)> GetIconF;

struct Popover {
    Wt::WString title;
    Wt::WString text;
    bool isValid() const {
        return !title.empty() && !text.empty();
    }
};

class MediaCollectionBrowserPrivate {
public:
    MediaCollectionBrowserPrivate(MediaCollection *collection, Settings *settings, Session *session, MediaCollectionBrowser *q)
        : collection(collection) , settings(settings), session(session), q(q) {}
    void rebuildBreadcrumb();
    void browse(boost::filesystem::path currentPath);
public:
    bool isAdmin = false;
    MediaCollection *const collection;
    Settings *settings;
    Session *session;
    boost::filesystem::path currentPath;
    Wt::WContainerWidget* breadcrumb;
    Wt::WContainerWidget* browser;
    Wt::Signal<Media> playSignal;
    Wt::Signal<Media> queueSignal;
    bool roleWasFetched = false;
private:
    void addDirectory(boost::filesystem::path directory);
    void addMedia(Media& media);
    Wt::WContainerWidget* addIcon(Wt::WString filename, GetIconF icon, MouseEventListener onClick, Popover popover = Popover());
    std::string formatFileSize(long size);
    void setTitleFor(Media media);
    void clearThumbnailsFor(Media media);
    void setPosterFor(Media media);
    MediaCollectionBrowser* q;
};
}


#endif
