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
#include <functional>
class MediaInfoPanel;

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

typedef std::function<void(Wt::WMouseEvent)> OnClick;

class RootMediaDirectory : public MediaDirectory
{
public:
    RootMediaDirectory(MediaCollection *mediaCollection);
    virtual std::vector< std::shared_ptr< MediaDirectory > > subDirectories() const;
private:
  MediaCollection *mediaCollection;
};

class MediaCollectionBrowser::Private {
public:
    Private(MediaCollection *collection, Settings *settings, Session *session, MediaCollectionBrowser *q);
    void rebuildBreadcrumb();
    void browse(const std::shared_ptr<MediaDirectory> &mediaDirectory);
    void setup(MediaInfoPanel *infoPanel);
    MediaCollection *const collection;
    Settings *settings;
    Session *session;
    std::shared_ptr<MediaDirectory> rootPath;
    std::shared_ptr<MediaDirectory> currentPath;
    Wt::WContainerWidget* breadcrumb;
    Wt::WContainerWidget* browser;
    Wt::Signal<Media> playSignal;
    Wt::Signal<Media> queueSignal;
    Wt::Signal<Media> infoRequested;
    Wt::Signal<> resetPanel;
    static std::string formatFileSize(long size);
    Wt::WPushButton* goToParent;
    typedef std::function<bool(const Media&, const Media&)> MediaSorter;
    enum Sort { Alpha, Date};
    enum SortDirection{ Asc, Desc };
    Sort sortBy = Alpha;
    SortDirection sortDirection = Asc;
    void setTitleFor(Media media);
    void clearThumbnailsFor(Media media);
    void clearAttachmentsFor(Media media);
    void setPosterFor(Media media);
private:
    void addDirectory( const std::shared_ptr< MediaDirectory > &directory );
    void addMedia(Media& media);
    Wt::WContainerWidget* addIcon(Wt::WString filename, GetIconF icon, OnClick onClick);
    MediaCollectionBrowser* q;
};


#endif
