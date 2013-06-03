#ifndef MEDIACOLLECTIONBROWSERPRIVATE_H
#define MEDIACOLLECTIONBROWSERPRIVATE_H

#include <boost/filesystem.hpp>
#include <Wt/WSignal>
#include <Wt/WContainerWidget>
#include <media.h>
#include <functional>
#include "Wt-Commons/wt_helpers.h"

namespace Wt {
class WPanel;
}

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
  WContainerWidget *labelValueBox(std::string label, Wt::WString value);
  WContainerWidget *labelValueBox(std::string label, Wt::WWidget *widget);
  Wt::Signal<Media> _play;
  Wt::Signal<Media> _queue;
  Wt::Signal<Media> _setTitle;
  Wt::Signal<Media> _setPoster;
  Wt::Signal<Media> _deletePoster;
  Session *session;
  Settings* settings;
  bool isAdmin;
  std::pair<Wt::WPanel*,Wt::WContainerWidget*> createPanel(std::string titleKey);
};

class CollectionPath;
typedef std::function<void(std::string key, CollectionPath *collectionPath)> OnDirectoryAdded;
typedef std::function<void(Media media)> OnMediaAdded;

class CollectionPath {
public:
  CollectionPath(CollectionPath *parent = 0) : parent_(parent) {}
  virtual void render(OnDirectoryAdded directoryAdded, OnMediaAdded mediaAdded ) = 0;
  virtual std::string label() const = 0;
  CollectionPath *parent() { return parent_; }
private:
  CollectionPath *parent_;
};

class DirectoryCollectionPath : public CollectionPath {
public:
  DirectoryCollectionPath(boost::filesystem::path path, MediaCollection *mediaCollection, CollectionPath *parent) : CollectionPath(parent), path(path), mediaCollection(mediaCollection) {}
  virtual void render(OnDirectoryAdded directoryAdded, OnMediaAdded mediaAdded);
  virtual std::string label() const;
private:
  boost::filesystem::path path;
  MediaCollection *mediaCollection;
};

class RootCollectionPath : public CollectionPath {
public:
  RootCollectionPath(Settings *settings, MediaCollection *mediaCollection) : CollectionPath(), settings(settings), mediaCollection(mediaCollection) {}
  virtual void render(OnDirectoryAdded directoryAdded, OnMediaAdded mediaAdded);
  virtual std::string label() const;
private:
  Settings *settings;
  MediaCollection *mediaCollection;
};

class MediaCollectionBrowserPrivate {
public:
    MediaCollectionBrowserPrivate(MediaCollection *collection, Settings *settings, Session *session, MediaCollectionBrowser *q)
        : collection(collection) , settings(settings), session(session), q(q) {}
    void rebuildBreadcrumb();
    void browse(CollectionPath *currentPath);
public:
    MediaCollection *const collection;
    Settings *settings;
    Session *session;
    CollectionPath *currentPath = 0;
    Wt::WContainerWidget* breadcrumb;
    Wt::WContainerWidget* browser;
    Wt::Signal<Media> playSignal;
    Wt::Signal<Media> queueSignal;
    InfoPanel* infoPanel;
    static std::string formatFileSize(long size);
    std::map<std::string,CollectionPath*> collectionPaths;
    
    
    void setTitleFor(Media media);
    void clearThumbnailsFor(Media media);
    void setPosterFor(Media media);
private:
    void addDirectory(CollectionPath *directory);
    void addMedia(Media& media);
    Wt::WContainerWidget* addIcon(Wt::WString filename, GetIconF icon, WtCommons::MouseEventListener onClick);
    MediaCollectionBrowser* q;
};
}


#endif
