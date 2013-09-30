/***********************************************************************
Copyright (c) 2013 "Marco Gulino <marco.gulino@gmail.com>"

This file is part of Pandorica: https://github.com/GuLinux/Pandorica

Pandorica is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details (included the COPYING file).

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/


#ifndef MEDIACOLLECTIONBROWSERPRIVATE_H
#define MEDIACOLLECTIONBROWSERPRIVATE_H

#include <boost/filesystem.hpp>
#include <Wt/WSignal>
#include <Wt/WContainerWidget>
#include "media/media.h"
#include <functional>
#include "Wt-Commons/wt_helpers.h"
#include "mediacollectionbrowser.h"

namespace Wt {
class WPanel;
class WTable;
}

class MediaCollectionBrowser;

class Session;
class Settings;
class MediaCollection;
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
    inline Wt::Signal<Media> &deleteAttachments() { return _deleteAttachments; }
    inline Wt::Signal<> &gotInfo() { return _gotInfo; }
    inline Wt::Signal<> &wasResetted() { return _wasResetted; }
    inline Wt::Signal<> &playFolder() { return _playFolder; }
    inline Wt::Signal<> &playFolderRecursive() { return _playFolderRecursive; }
private:
  void labelValueBox(std::string label, Wt::WString value, Wt::WTable* container);
  void labelValueBox(std::string label, Wt::WWidget *widget, Wt::WTable* container);
  Wt::Signal<Media> _play;
  Wt::Signal<Media> _queue;
  Wt::Signal<Media> _setTitle;
  Wt::Signal<Media> _setPoster;
  Wt::Signal<Media> _deletePoster;
  Wt::Signal<Media> _deleteAttachments;
  Wt::Signal<> _gotInfo;
  Wt::Signal<> _wasResetted;
  Wt::Signal<> _playFolder;
  Wt::Signal<> _playFolderRecursive;
  Session *session;
  Settings* settings;
  bool isAdmin;
  std::pair<Wt::WPanel*,Wt::WContainerWidget*> createPanel(std::string titleKey);
};

class InfoPanelMultiplex : public Wt::WObject {
public:
  InfoPanelMultiplex(Wt::WObject* parent = 0) : Wt::WObject(parent) {}
  void info(Media media);
  void reset();
  inline Wt::Signal<Media> &play() { return _play; }
  inline Wt::Signal<Media> &queue() { return _queue; }
  inline Wt::Signal<Media> &setTitle() { return _setTitle; }
  inline Wt::Signal<Media> &setPoster() { return _setPoster; }
  inline Wt::Signal<Media> &deletePoster() { return _deletePoster; }
  inline Wt::Signal<Media> &deleteAttachments() { return _deleteAttachments; }
  InfoPanel *add(InfoPanel *panel);
  void setup();
private:
  Wt::Signal<Media> _play;
  Wt::Signal<Media> _queue;
  Wt::Signal<Media> _setTitle;
  Wt::Signal<Media> _setPoster;
  Wt::Signal<Media> _deletePoster;
  Wt::Signal<Media> _deleteAttachments;
  std::vector<InfoPanel*> panels;
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
  virtual bool hasMedia(Media &media) = 0;
  virtual bool hasMediaInSubPath(Media &media) = 0;
private:
  CollectionPath *parent_;
};

class DirectoryCollectionPath : public CollectionPath {
public:
  DirectoryCollectionPath(boost::filesystem::path path, MediaCollection *mediaCollection, CollectionPath *parent) : CollectionPath(parent), path(path), mediaCollection(mediaCollection) {}
  virtual void render(OnDirectoryAdded directoryAdded, OnMediaAdded mediaAdded);
  virtual std::string label() const;
  virtual bool hasMedia(Media& media);
  virtual bool hasMediaInSubPath(Media& media);
private:
  boost::filesystem::path path;
  MediaCollection *mediaCollection;
};

class RootCollectionPath : public CollectionPath {
public:
  RootCollectionPath(Settings *settings, Session *session, MediaCollection *mediaCollection) : CollectionPath(), settings(settings), session(session), mediaCollection(mediaCollection) {}
  virtual void render(OnDirectoryAdded directoryAdded, OnMediaAdded mediaAdded);
  virtual std::string label() const;
  virtual bool hasMedia(Media& media) { return false; }
  virtual bool hasMediaInSubPath(Media& media) { return true; }
private:
  Settings *settings;
  MediaCollection *mediaCollection;
  Session* session;
};

typedef std::function<void(Wt::WMouseEvent)> OnClick;

class MediaCollectionBrowser::Private {
public:
    Private(MediaCollection *collection, Settings *settings, Session *session, MediaCollectionBrowser *q)
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
    InfoPanelMultiplex* infoPanel;
    static std::string formatFileSize(long size);
    std::map<std::string,CollectionPath*> collectionPaths;
    Wt::WPushButton* goToParent;
    
    
    void setTitleFor(Media media);
    void clearThumbnailsFor(Media media);
    void clearAttachmentsFor(Media media);
    void setPosterFor(Media media);
private:
    void addDirectory(CollectionPath *directory);
    void addMedia(Media& media);
    Wt::WContainerWidget* addIcon(Wt::WString filename, GetIconF icon, OnClick onClick);
    MediaCollectionBrowser* q;
};


#endif
