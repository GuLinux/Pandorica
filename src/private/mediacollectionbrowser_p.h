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
#include <savemediainformation.h>
#include <savemediathumbnail.h>
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
class FlatMediaDirectory : public MediaDirectory
{
public:
    FlatMediaDirectory(MediaCollection *mediaCollection);
    virtual std::vector< std::shared_ptr< MediaDirectory > > subDirectories() const;
    virtual std::vector< Media > medias() const;
    virtual std::vector< Media > allMedias() const;
private:
  MediaCollection *mediaCollection;
};

class MediaCollectionBrowser::Private {
public:
    Private(MediaCollection *collection, Settings *settings, Session *session, MediaCollectionBrowser *q);
    void rebuildBreadcrumb();
    void browse(const std::shared_ptr<MediaDirectory> &mediaDirectory, bool forceReload = false);
    void setup(MediaInfoPanel *infoPanel);
    MediaCollection *const collection;
    Settings *settings;
    Session *session;
    std::shared_ptr<MediaDirectory> rootPath;
    std::shared_ptr<MediaDirectory> flatPath;
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
    enum Sort { Alpha, Date, Rating, Size};
    enum SortDirection{ Asc, Desc };
    Sort sortBy = Alpha;
    SortDirection sortDirection = Asc;
    void setTitleFor(Media media, std::function< void(const Media &) > refreshIcon);
    void clearThumbnailsFor(Media media, std::function< void(const Media &) > refreshIcon);
    void clearAttachmentsFor(Media media, std::function< void(const Media &) > refreshIcon);
    void setPosterFor(Media media, std::function< void(const Media &) > refreshIcon);
    typedef std::function<bool(Wt::Dbo::Transaction &, const Media&)> MediaFilter;
    std::map<Wt::WMenuItem*, MediaFilter> mediaFilters;
    void titleFilterDialog(Wt::WMenu *menu);
    void dateFilterDialog(Wt::WMenu *menu);
    void ratingFilterDialog(Wt::WMenu *menu);
    struct FilterDialogResult {
      Wt::WString menuItemTitle;
      MediaFilter mediaFilter;
    };
    void addFilterDialog( const Wt::WString &title, Wt::WWidget *content, std::function<FilterDialogResult()> onOkClicked, std::function<void(Wt::WPushButton*)> okButtonEnabler, Wt::WMenu *menu );
    SaveMediaInformation saveMediaInformation;
    SaveMediaThumbnail saveMediaThumbnail;
    void clear();
    bool empty = true;
private:
    void addDirectory( const std::shared_ptr< MediaDirectory > &directory );
    void addMedia(const Media& media, Wt::WContainerWidget *widget = 0);
    Wt::WContainerWidget* addIcon(Wt::WString filename, GetIconF icon, OnClick onClick);
    Wt::WContainerWidget* replaceIcon(Wt::WString filename, GetIconF icon, OnClick onClick, Wt::WContainerWidget *existing);
    MediaCollectionBrowser* q;
};



#endif
