#ifndef MEDIACOLLECTIONBROWSERPRIVATE_H
#define MEDIACOLLECTIONBROWSERPRIVATE_H

#include <boost/filesystem.hpp>
#include <Wt/WSignal>
#include <Wt/WContainerWidget>
#include <media.h>
#include "Wt-Commons/wt_helpers.h"

class MediaCollectionBrowser;

class Session;
class Settings;
class MediaCollection;
namespace StreamingPrivate {
typedef std::pair<std::string,Media> MediaEntry;
typedef std::function<std::string(Wt::WObject*)> GetIconF;

class InfoPanel : public Wt::WContainerWidget {
public:
    InfoPanel(Session* session, Settings* settings, Wt::WContainerWidget* parent = 0);
    void info(Media media);
    void reset();
    inline Wt::Signal<Media> &play() { return _play; }
    inline Wt::Signal<Media> &queue() { return _queue; }
    inline Wt::Signal<Media> &setTitle() { return _setTitle; }
    inline Wt::Signal<Media> &setPoster() { return _setPoster; }
    inline Wt::Signal<Media> &deletePoster() { return _deletePoster; }
private:
  WContainerWidget *labelValueBox(Wt::WString label, Wt::WString value);
  Wt::Signal<Media> _play;
  Wt::Signal<Media> _queue;
  Wt::Signal<Media> _setTitle;
  Wt::Signal<Media> _setPoster;
  Wt::Signal<Media> _deletePoster;
  Session *session;
  Settings* settings;
  bool isAdmin;
};


class MediaCollectionBrowserPrivate {
public:
    MediaCollectionBrowserPrivate(MediaCollection *collection, Settings *settings, Session *session, MediaCollectionBrowser *q)
        : collection(collection) , settings(settings), session(session), q(q) {}
    void rebuildBreadcrumb();
    void browse(boost::filesystem::path currentPath);
public:
    MediaCollection *const collection;
    Settings *settings;
    Session *session;
    boost::filesystem::path currentPath;
    Wt::WContainerWidget* breadcrumb;
    Wt::WContainerWidget* browser;
    Wt::Signal<Media> playSignal;
    Wt::Signal<Media> queueSignal;
    InfoPanel* infoPanel;
    static std::string formatFileSize(long size);
    
    
    void setTitleFor(Media media);
    void clearThumbnailsFor(Media media);
    void setPosterFor(Media media);
private:
    void addDirectory(boost::filesystem::path directory);
    void addMedia(Media& media);
    Wt::WContainerWidget* addIcon(Wt::WString filename, GetIconF icon, MouseEventListener onClick);
    MediaCollectionBrowser* q;
};
}


#endif
