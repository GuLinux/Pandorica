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


#include "mediacollectionbrowser.h"
#include <Wt/WMenu>
#include <Wt/WAnchor>
#include <Wt/WText>
#include <Wt/WPopupMenu>
#include <Wt/WImage>
#include <Wt/WMemoryResource>
#include <Wt/WCalendar>
#include "Wt-Commons/wt_helpers.h"
#include "Wt-Commons/wt_helpers.h"
#include "settings.h"
#include "session.h"
#include "utils/utils.h"

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <algorithm>
#include <Wt/Dbo/Transaction>
#include <Wt/WTime>
#include <Wt/WMessageBox>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WFileResource>
#include <Wt/WVBoxLayout>
#include <Wt/WHBoxLayout>
#include "private/mediacollectionbrowser_p.h"
#include "Models/models.h"
#include "MediaScanner/createthumbnails.h"
#include "ffmpegmedia.h"
#include "media/mediadirectory.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include <Wt/WPanel>
#include <Wt/WGroupBox>
#include <Wt/WTimer>
#include <Wt/WTable>
#include <Wt/WToolBar>
#include <Wt/WCheckBox>
#include <Wt/WSlider>
#include <Wt/WComboBox>
#include "utils/d_ptr_implementation.h"
#include "mediainfopanel.h"


using namespace Wt;
using namespace std;
namespace fs = boost::filesystem;
using namespace WtCommons;

MediaCollectionBrowser::Private::Private(MediaCollection *collection, Settings *settings, Session *session, MediaCollectionBrowser *q)
  : collection(collection) , settings(settings), session(session), q(q), rootPath(new RootMediaDirectory(collection)), flatPath(new FlatMediaDirectory(collection)), currentPath(rootPath)
{
  rootPath->setLabel(wtr( "mediacollection.root" ).toUTF8());
}

MediaCollectionBrowser::MediaCollectionBrowser( MediaCollection *collection, Settings *settings, Session *session, WContainerWidget *parent )
  : WContainerWidget( parent ), d(collection, settings, session, this )
{
  d->breadcrumb = WW<WContainerWidget>().css( "breadcrumb visible-desktop inline-breadcrumb" );
  d->breadcrumb->setList( true );
  d->browser = WW<WContainerWidget>().css( "thumbnails" ).setMargin( WLength::Auto, Left ).setMargin( WLength::Auto, Right );
  WContainerWidget *mainContainer = new WContainerWidget;
  MediaInfoPanel *mobileMediaInfoPanelWidget = WW<MediaInfoPanel>( session, settings ) ;
  mobileMediaInfoPanelWidget->setHidden( true );
  mobileMediaInfoPanelWidget->gotInfo().connect( [ = ]( _n6 )
  {
    settings->animateShow( Settings::ShowMediaInfoAnimation, mobileMediaInfoPanelWidget );
  } );
  mobileMediaInfoPanelWidget->wasResetted().connect( [ = ]( _n6 )
  {
    settings->animateHide( Settings::HideMediaInfoAnimation, mobileMediaInfoPanelWidget );
  } );

  WContainerWidget *container = WW<WContainerWidget>( mainContainer ).css( "container-fluid" );


  WContainerWidget *row = WW<WContainerWidget>( container ).css( "row-fluid" );
  MediaInfoPanel *desktopMediaInfoPanel = WW<MediaInfoPanel>( session, settings ).addCss( "visible-desktop span4" );
  row->addWidget( desktopMediaInfoPanel );
  row->addWidget( WW<WContainerWidget>().css( "mediabrowser span8" ).add( d->browser ) );


  d->browser->setList( true );
  WContainerWidget *breadcrumb = WW<WContainerWidget>().css("breadcrumb visible-desktop");
  WPushButton *reloadButton = WW<WPushButton>( wtr( "mediacollection.reload" ) ).css( "btn btn-small" ).onClick([=](WMouseEvent)
  {
    Dbo::Transaction t( *session );
    collection->rescan( t );
    d->browse( d->currentPath == d->flatPath ? d->flatPath : d->rootPath );
  } );
  WPushButton *sortByButton = WW<WPushButton>(wtr("mediacollectionbrowser_sort_by")).css("btn btn-small").setMenu(new WPopupMenu);
  shared_ptr<vector<WMenuItem*>> sortMenuGroup(new vector<WMenuItem*>);
  shared_ptr<vector<WMenuItem*>> sortOrderMenuGroup(new vector<WMenuItem*>);
  auto addCheckableItem = [=](WMenu *menu, const WString &label, function<void()> onClick, const shared_ptr<vector<WMenuItem*>> &group) {
    WMenuItem *item = menu->addItem(label);
    item->setCheckable(true);
    item->triggered().connect([=](WMenuItem*,_n5){
      for(auto otherItem : *group)
        otherItem->setChecked(false);
      item->setChecked(true);
      onClick();
    });
    group->push_back(item);
    return item;
  };
  addCheckableItem(sortByButton->menu(), wtr("mediacollectionbrowser_file_name"), [=]{d->sortBy = Private::Alpha; reload(); }, sortMenuGroup)->setChecked(d->sortBy == Private::Alpha);
  addCheckableItem(sortByButton->menu(), wtr("mediacollectionbrowser_date_added"), [=]{d->sortBy = Private::Date; reload(); }, sortMenuGroup)->setChecked(d->sortBy == Private::Date);
  addCheckableItem(sortByButton->menu(), wtr("mediacollectionbrowser_rating"), [=]{d->sortBy = Private::Rating; reload(); }, sortMenuGroup)->setChecked(d->sortBy == Private::Rating);
  sortByButton->menu()->addSeparator();
  addCheckableItem(sortByButton->menu(), wtr("mediacollectionbrowser_ascending"), [=]{d->sortDirection = Private::Asc; reload(); }, sortOrderMenuGroup)->setChecked(d->sortDirection == Private::Asc);
  addCheckableItem(sortByButton->menu(), wtr("mediacollectionbrowser_descending"), [=]{d->sortDirection = Private::Desc; reload(); }, sortOrderMenuGroup)->setChecked(d->sortDirection == Private::Desc);
  sortByButton->setMenu(sortByButton->menu());

  shared_ptr<vector<WMenuItem*>> viewModeItems(new vector<WMenuItem*>);
  WPushButton *viewModeButton = WW<WPushButton>(wtr("mediacollectionbrowser_view_mode")).css("btn btn-small").setMenu(new WPopupMenu);
  
  
  addCheckableItem(viewModeButton->menu(), wtr("mediacollectionbrowser_filesystem_view"), [=]{ d->browse(d->rootPath); }, viewModeItems)->setChecked(true);
  addCheckableItem(viewModeButton->menu(), wtr("mediacollectionbrowser_all_medias"), [=]{ d->browse(d->flatPath); }, viewModeItems);

  WPushButton *filtersButton = WW<WPushButton>(wtr("mediacollectionbrowser_filters")).css("btn btn-small").setMenu(new WPopupMenu);
  filtersButton->menu()->itemClosed().connect([=](WMenuItem *item, _n5) { d->mediaFilters.erase(item); reload(); });
  auto addFilterMenuItem = filtersButton->menu()->addSectionHeader(wtr("mediacollectionbrowser_filters_add_filter"));
  filtersButton->menu()->addItem(wtr("mediacollectionbrowser_filters_name"))->triggered().connect([=](WMenuItem*, _n5){ d->titleFilterDialog(filtersButton->menu()); });
//   filtersButton->menu()->addItem("Filename");
  filtersButton->menu()->addItem(wtr("mediacollectionbrowser_filters_date"))->triggered().connect([=](WMenuItem*, _n5){ d->dateFilterDialog(filtersButton->menu()); });
  filtersButton->menu()->addItem(wtr("mediacollectionbrowser_filters_rating"))->triggered().connect([=](WMenuItem*, _n5){ d->ratingFilterDialog(filtersButton->menu()); });
  filtersButton->menu()->addSeparator();
  filtersButton->menu()->addSectionHeader(wtr("mediacollectionbrowser_filters_current_filters"));
  breadcrumb->addWidget(WW<WToolBar>().addButton(reloadButton).addButton(viewModeButton).addButton(sortByButton).addButton(filtersButton));
  breadcrumb->addWidget(d->breadcrumb);
  addWidget( breadcrumb );
  addWidget( d->goToParent = WW<WPushButton>( wtr( "button.parent.directory" ) ).css( "btn btn-block hidden-desktop" ).onClick( [ = ]( WMouseEvent )
  {
    d->browse( d->rootPath );
  } ) );
  addWidget( WW<WContainerWidget>().css( "hidden-desktop" ).add( mobileMediaInfoPanelWidget ) );
  addWidget( mainContainer );
  collection->scanned().connect( this, &MediaCollectionBrowser::reload );
  d->setup(desktopMediaInfoPanel);
  d->setup(mobileMediaInfoPanelWidget);
}


void MediaCollectionBrowser::Private::addFilterDialog( const WString &title, WWidget *content, function<FilterDialogResult()> onOkClicked, std::function<void(Wt::WPushButton*)> okButtonEnabler, WMenu *menu )
{
  WDialog *dialog = new WDialog(title);
  dialog->setModal(true);
  dialog->setClosable(true);
  dialog->contents()->addWidget(content);
  WPushButton *okButton;
  dialog->footer()->addWidget(okButton = WW<WPushButton>(wtr("button.ok")).css("btn btn-primary").onClick([=](WMouseEvent){
    FilterDialogResult result = onOkClicked();
    auto menuItem = menu->addItem(result.menuItemTitle);
    menuItem->setCloseable(true);
    menuItem->addStyleClass("menuitem-closeable");
    mediaFilters[menuItem] = result.mediaFilter;
    dialog->done(WDialog::DialogCode::Accepted);
    q->reload();
  }));
  okButtonEnabler(okButton);
  dialog->show();
}


void MediaCollectionBrowser::Private::titleFilterDialog( WMenu *menu )
{
  WLineEdit *titleLineEdit = WW<WLineEdit>().css("input-block-level");
  titleLineEdit->setEmptyText("insert title filter");
  WCheckBox *caseSensitiveCheckbox = WW<WCheckBox>("Case sensitive").css("input-block-level");
  
  auto onDialogOk = [=]{
    return FilterDialogResult{
      wtr("mediacollectionbrowser_filters_name_menuitem").arg(titleLineEdit->text()),
      [=] (Dbo::Transaction &t, const Media &media) {
        string filter = titleLineEdit->text().toUTF8();
        string mediaTitle = media.title(t).toUTF8();
        return caseSensitiveCheckbox->checkState() == Wt::Checked ? boost::algorithm::contains(mediaTitle, filter) : boost::algorithm::icontains(mediaTitle, filter) ;
      }
    };
  };
  
  addFilterDialog(
    wtr("mediacollectionbrowser_filters_name_dialogtitle"),
    WW<WContainerWidget>().add(titleLineEdit).add(caseSensitiveCheckbox),
    onDialogOk,
    [=](WPushButton *okButton){
      titleLineEdit->changed().connect([=](_n1){ okButton->setEnabled(!titleLineEdit->text().empty()); });
      titleLineEdit->keyWentUp().connect([=](WKeyEvent) { okButton->setEnabled(!titleLineEdit->text().empty()); } );
      okButton->disable();
    },
    menu
  );
}

void MediaCollectionBrowser::Private::dateFilterDialog( WMenu *menu )
{
  WCalendar *calendar = new WCalendar();
  calendar->select(WDate::currentDate());
  WComboBox *dateFilterType = WW<WComboBox>().addCss("input-block-level");
  dateFilterType->addItem(wtr("mediacollectionbrowser_filters_dategt_menu"));
  dateFilterType->addItem(wtr("mediacollectionbrowser_filters_datelt_menu"));
  auto onDialogOk = [=]{
    int dateFilterTypeIndex = dateFilterType->currentIndex();
    return FilterDialogResult {
      wtr(dateFilterTypeIndex>0 ? "mediacollectionbrowser_filters_datelt_menuitem" : "mediacollectionbrowser_filters_dategt_menuitem")
        .arg(calendar->selection().begin()->toString()),
      [=] (Dbo::Transaction &t, const Media &media) {
        if(dateFilterTypeIndex == 0)
          return media.creationTime(t).date() >= *calendar->selection().begin();
        return media.creationTime(t).date() <= *calendar->selection().begin();
      }
    };
  };
  addFilterDialog(
    wtr("mediacollectionbrowser_filters_bydate_dialogtitle"),
    WW<WContainerWidget>().add(dateFilterType).add(calendar),
    onDialogOk,
    [=](WPushButton *okButton){ okButton->enable(); },
    menu
  );
}

void MediaCollectionBrowser::Private::ratingFilterDialog( WMenu *menu )
{
  WComboBox *ratingFilterType = WW<WComboBox>().addCss("input-block-level");
  ratingFilterType->addItem(wtr("mediacollectionbrowser_filters_ratinggt_menu"));
  ratingFilterType->addItem(wtr("mediacollectionbrowser_filters_ratinglt_menu"));
  WSlider *ratingSlider = new WSlider(Wt::Horizontal);
  ratingSlider->setMinimum(0);
  ratingSlider->setMaximum(5);
  ratingSlider->setTickInterval(1);
  WText *currentValue = WW<WText>(ratingSlider->valueText()).setMargin(5, Side::Left);
  ratingSlider->valueChanged().connect([=](int rating, _n5){ currentValue->setText(boost::lexical_cast<string>(rating));});
  
  auto onDialogOk = [=] {
    int ratingFilterTypeIndex = ratingFilterType->currentIndex();
    return FilterDialogResult {
      wtr( ratingFilterTypeIndex >0 ? "mediacollectionbrowser_filters_ratinglt_menuitem" : "mediacollectionbrowser_filters_ratinggt_menuitem").arg(ratingSlider->value()),
      [=] (Dbo::Transaction &t, const Media &media) {
        if(ratingFilterTypeIndex == 0)
          return MediaRating::ratingFor(media, t).ratingAverage >= ratingSlider->value();
        return MediaRating::ratingFor(media, t).ratingAverage <= ratingSlider->value();
      }
    };
  };
  
  addFilterDialog(
    wtr("mediacollectionbrowser_filters_by_rating_dialogtitle"),
    WW<WContainerWidget>().add(ratingFilterType).add(ratingSlider).add(currentValue),
    onDialogOk,
    [=](WPushButton *okButton){ okButton->enable(); },
    menu
  );
}


void MediaCollectionBrowser::reload()
{
  d->browse( d->currentPath );
}

void MediaCollectionBrowser::Private::setup(MediaInfoPanel *infoPanel)
{
  infoPanel->play().connect( [ = ]( Media media, _n5 )
  {
    playSignal.emit( media );
  } );
  infoPanel->queue().connect( [ = ]( Media media, _n5 )
  {
    queueSignal.emit( media );
  } );
  infoPanel->setTitle().connect( this, &MediaCollectionBrowser::Private::setTitleFor );
  infoPanel->setPoster().connect( this, &MediaCollectionBrowser::Private::setPosterFor );
  infoPanel->deletePoster().connect( this, &MediaCollectionBrowser::Private::clearThumbnailsFor );
  infoPanel->deleteAttachments().connect( this, &MediaCollectionBrowser::Private::clearAttachmentsFor);
  infoPanel->playFolder().connect( [ = ]( _n6 )
  {
    for( Media media : collection->sortedMediasList() )
    {
      if( currentPath->contains(media) )
        queueSignal.emit( media );
    }
  } );
  infoPanel->playFolderRecursive().connect( [ = ]( _n6 )
  {
    for( Media media : collection->sortedMediasList() )
    {
      if( currentPath->recursiveContains(media) )
        queueSignal.emit( media );
    }
  } );
  infoRequested.connect([=](Media &media, _n5) { infoPanel->info(media);});
  resetPanel.connect([=](_n6) { infoPanel->reset(); });
}

void MediaCollectionBrowser::Private::browse( const shared_ptr< MediaDirectory > &mediaDirectory )
{
  currentPath = mediaDirectory;
  resetPanel.emit();
  browser->clear();
  rebuildBreadcrumb();
  for(auto dir: currentPath->subDirectories()) {
    addDirectory(dir);
  }
  Dbo::Transaction t(*session);
  map<Sort, MediaSorter> sorters {
    {Sort::Alpha, [](const Media &_1, const Media &_2) { return _1.filename() < _2.filename();}},
    {Sort::Date, [&t](const Media &_1, const Media &_2) { return _1.creationTime(t) < _2.creationTime(t); }},
    {Sort::Rating, [&t](const Media &_1, const Media &_2) {
      return MediaRating::ratingFor(_1, t).score() < MediaRating::ratingFor(_2, t).score(); }},
  };
  vector<Media> medias = currentPath->medias();
  if(sortDirection == Asc)
  sort(begin(medias), end(medias), sorters[sortBy]);
  else
    sort(medias.rbegin(), medias.rend(), sorters[sortBy]);
  for(auto media: medias)
    addMedia(media);
}


RootMediaDirectory::RootMediaDirectory( MediaCollection *mediaCollection ): MediaDirectory(fs::path()), mediaCollection(mediaCollection)
{
}

vector< shared_ptr< MediaDirectory > > RootMediaDirectory::subDirectories() const
{
  return mediaCollection->rootDirectories();
}


FlatMediaDirectory::FlatMediaDirectory( MediaCollection *mediaCollection ): MediaDirectory( fs::path() ), mediaCollection(mediaCollection)
{
}
std::vector< Media > FlatMediaDirectory::allMedias() const
{
  auto collection = mediaCollection->collection();
  vector<Media> all;
  transform(begin(collection), end(collection), back_insert_iterator<vector<Media>>(all), [](const pair<string,Media> &p){ return p.second; });
  return all;
}

std::vector< Media > FlatMediaDirectory::medias() const
{
  return allMedias();
}
std::vector< std::shared_ptr< MediaDirectory > > FlatMediaDirectory::subDirectories() const
{
  return {};
}



bool MediaCollectionBrowser::currentDirectoryHas( const Media &media ) const
{
  return d->currentPath->contains(media);
}



void MediaCollectionBrowser::Private::addDirectory( const shared_ptr<MediaDirectory> &directory )
{
  if(directory->allMedias().empty())
    return;
  auto onClick = [=]( WMouseEvent )
  {
    browse( directory );
  };
  addIcon( directory->label(), []( WObject * )
  {
    return Settings::icon( Settings::FolderBig );
  }, onClick );
}


void MediaCollectionBrowser::Private::addMedia( Media &media )
{
  wApp->log( "notice" ) << "adding media " << media.path();
  Dbo::Transaction t( *session );

  for(auto filter: mediaFilters)
    if(! filter.second(t, media))
      return;
  auto onClick = [ = ]( WMouseEvent e )
  {
    infoRequested.emit(media);
  };

  GetIconF icon = []( WObject * )
  {
    return Settings::icon( Settings::VideoFile );
  };

  if( media.mimetype().find( "audio" ) != string::npos )
    icon = []( WObject * )
  {
    return Settings::icon( Settings::AudioFile );
  };

  Dbo::ptr<MediaAttachment> preview = media.preview( t, Media::PreviewThumb );

  if( preview )
    icon = [ = ]( WObject * parent )
  {
    Dbo::Transaction t( *session );
    return preview->link( preview, t, parent ).url();
  };

  addIcon( media.title( t ), icon, onClick );
}

void MediaCollectionBrowser::Private::clearThumbnailsFor( Media media )
{
  Dbo::Transaction t( *session );
  session->execute( "DELETE FROM media_attachment WHERE media_id=? and type = 'preview';" ).bind( media.uid() );
  t.commit();
  q->reload();
}

void MediaCollectionBrowser::Private::clearAttachmentsFor( Media media )
{
  Dbo::Transaction t( *session );
  session->execute( "DELETE FROM media_attachment WHERE media_id=?;" ).bind( media.uid() );
  t.commit();
  q->reload();
}


void MediaCollectionBrowser::Private::setPosterFor( Media media )
{
  shared_ptr<FFMPEGMedia> ffmpegMedia (new FFMPEGMedia {media, [=](const string &level) { return wApp->log(level); } });
  WDialog *dialog = new WDialog( wtr( "mediabrowser.admin.setposter" ) );
  auto createThumbs = new CreateThumbnails {wApp, settings, dialog};
  dialog->footer()->addWidget( WW<WPushButton>( wtr( "button.cancel" ) ).css( "btn btn-danger" ).onClick( [ = ]( WMouseEvent )
  {
    dialog->reject();
  } ) );
  dialog->footer()->addWidget( WW<WPushButton>( wtr( "button.ok" ) ).css( "btn btn-success" ).onClick( [ = ]( WMouseEvent )
  {
    Dbo::Transaction t( *session );
    createThumbs->save( &t );
    t.commit();
    dialog->accept();
    q->reload();
  } ) );
  dialog->show();
  dialog->resize( 500, 500 );
  auto runStep = [ = ]
  {
    Dbo::Transaction t( *session );
    createThumbs->run( ffmpegMedia.get(), media, dialog->contents(), &t, MediaScannerStep::OverwriteIfExisting );
  };
  createThumbs->redo().connect( [ = ]( _n6 )
  {
    runStep();
  } );
  runStep();
}


void MediaCollectionBrowser::Private::setTitleFor( Media media )
{
  Dbo::Transaction t( *session );
  MediaPropertiesPtr properties = media.properties( t );

  if( !properties )
  {
    t.rollback();
    WMessageBox::show( wtr( "mediabrowser.admin.settitle.missingproperties.caption" ), wtr( "mediabrowser.admin.settitle.missingproperties.body" ), StandardButton::Ok );
    return;
  }

  WDialog *setTitleDialog = new WDialog( wtr( "mediabrowser.admin.settitle" ) );
  setTitleDialog->contents()->addStyleClass( "form-inline" );
  WLineEdit *titleEdit = new WLineEdit( properties->title().empty() ? media.filename() : properties->title() );
  WPushButton *okButton = WW<WPushButton>( "Ok" ).onClick( [ = ]( WMouseEvent )
  {
    setTitleDialog->accept();
  } ).css( "btn" );
  auto editIsEnabled = [ = ]
  {
    return !titleEdit->text().empty() && titleEdit->text().toUTF8() != media.filename() && titleEdit->text().toUTF8() != properties->title();
  };
  okButton->setEnabled( editIsEnabled() );

  titleEdit->keyWentUp().connect( [ = ]( WKeyEvent key )
  {
    if( key.key() == Wt::Key_Enter && editIsEnabled() )
      setTitleDialog->accept();

    okButton->setEnabled( editIsEnabled() );
  } );
  setTitleDialog->contents()->addWidget( new WText {wtr( "set.title.filename.hint" )} );
  string titleHint = Utils::titleHintFromFilename( media.filename() );
  setTitleDialog->contents()->addWidget( WW<WAnchor>( "", titleHint ).css( "link-hand" ).onClick( [ = ]( WMouseEvent )
  {
    titleEdit->setText( titleHint );
    okButton->setEnabled( editIsEnabled() );
  } ) );
  setTitleDialog->contents()->addWidget( new WBreak );
  setTitleDialog->contents()->addWidget( titleEdit );
  setTitleDialog->contents()->addWidget( okButton );
  setTitleDialog->contents()->setPadding( 10 );
  setTitleDialog->setClosable( true );
  titleEdit->setWidth( 500 );
  setTitleDialog->finished().connect( [ = ]( WDialog::DialogCode code, _n5 )
  {
    if( code != WDialog::Accepted )
      return;

    Dbo::Transaction t( *session );
    properties.modify()->setTitle( titleEdit->text().toUTF8() );
    properties.flush();
    t.commit();
    q->reload();
  } );
  setTitleDialog->show();
}


WContainerWidget *MediaCollectionBrowser::Private::addIcon( WString filename, GetIconF icon, OnClick onClick )
{
  WContainerWidget *item = WW<WContainerWidget>().css( "span3 media-icon-container" );
  item->setContentAlignment( AlignmentFlag::AlignCenter );
  WAnchor *link = WW<WAnchor>( "#" ).css( "thumbnail filesystem-item link-hand" );
  link->setImage( new WImage( icon( item ) ) );
  link->addWidget( WW<WText>( filename ).css( "filesystem-item-label" ) );
  item->addWidget( link );
  link->clicked().connect( onClick );

  browser->addWidget( item );
  return item;
}




void MediaCollectionBrowser::Private::rebuildBreadcrumb()
{
  breadcrumb->clear();
  if(currentPath == flatPath)
    return;
  list<shared_ptr<MediaDirectory>> paths;
  shared_ptr<MediaDirectory> current = currentPath;

  while( current && current != rootPath )
  {
    paths.push_front( current );
    current = current->parent();
  }

  paths.push_front(rootPath);
  goToParent->setEnabled( paths.size() > 1 );

  for( shared_ptr<MediaDirectory> path : paths )
  {
    WContainerWidget *item = new WContainerWidget;

    if( breadcrumb->count() )
      item->addWidget( WW<WText>( "/" ).css( "divider" ) );

    item->addWidget( WW<WAnchor>( "", path->label() ).css( "link-hand" ).onClick( [ = ]( WMouseEvent )
    {
      browse( path );
    } ) );
    breadcrumb->addWidget( item );
  }
}

Signal< Media > &MediaCollectionBrowser::play()
{
  return d->playSignal;
}

Signal< Media > &MediaCollectionBrowser::queue()
{
  return d->queueSignal;
}



MediaCollectionBrowser::~MediaCollectionBrowser()
{
}

