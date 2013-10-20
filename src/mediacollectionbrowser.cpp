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
#include "Wt-Commons/wt_helpers.h"
#include "Wt-Commons/wt_helpers.h"
#include "settings.h"
#include "session.h"
#include "utils/utils.h"

#include <boost/format.hpp>
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
#include "utils/d_ptr_implementation.h"
#include "mediainfopanel.h"


#define ROOT_PATH_ID "///ROOT///"

using namespace Wt;
using namespace std;
using namespace boost;
namespace fs = boost::filesystem;
using namespace WtCommons;


MediaCollectionBrowser::MediaCollectionBrowser( MediaCollection *collection, Settings *settings, Session *session, WContainerWidget *parent )
  : WContainerWidget( parent ), d(collection, settings, session, this )
{
  d->breadcrumb = WW<WContainerWidget>().css( "breadcrumb visible-desktop" );
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
  addWidget( d->breadcrumb );
  addWidget( d->goToParent = WW<WPushButton>( wtr( "button.parent.directory" ) ).css( "btn btn-block hidden-desktop" ).onClick( [ = ]( WMouseEvent )
  {
    d->browse( d->currentPath->parent() );
  } ) );
  addWidget( WW<WContainerWidget>().css( "hidden-desktop" ).add( mobileMediaInfoPanelWidget ) );
  addWidget( mainContainer );
  d->currentPath = new RootCollectionPath {settings, session, collection};
  d->collectionPaths[ROOT_PATH_ID] = d->currentPath;
  collection->scanned().connect( this, &MediaCollectionBrowser::reload );
  d->setup(desktopMediaInfoPanel);
  d->setup(mobileMediaInfoPanelWidget);
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
      if( currentPath->hasMedia( media ) )
        queueSignal.emit( media );
    }
  } );
  infoPanel->playFolderRecursive().connect( [ = ]( _n6 )
  {
    for( Media media : collection->sortedMediasList() )
    {
      if( currentPath->hasMediaInSubPath( media ) )
        queueSignal.emit( media );
    }
  } );
  infoRequested.connect([=](Media &media, _n5) { infoPanel->info(media);});
  resetPanel.connect([=](_n6) { infoPanel->reset(); });
}

void MediaCollectionBrowser::Private::browse( CollectionPath *currentPath )
{
  this->currentPath = currentPath;
  resetPanel.emit();
  browser->clear();
  rebuildBreadcrumb();
  currentPath->render( [ = ]( string key, CollectionPath * collectionPath )
  {
    if( collectionPaths[key] )
    {
      delete collectionPaths[key];
      collectionPaths.erase( key );
    }

    collectionPaths[key] = collectionPath;
    addDirectory( collectionPath );
  }, [ = ]( Media media )
  {
    addMedia( media );
  } );
}

bool MediaCollectionBrowser::currentDirectoryHas( Media &media ) const
{
  return d->currentPath->hasMedia( media );
}




void DirectoryCollectionPath::render( OnDirectoryAdded directoryAdded, OnMediaAdded mediaAdded )
{
  MediaDirectory mediaDirectory{path};

  for( MediaEntry m : mediaCollection->collection() )
  {
    mediaDirectory.add(m.second);
  }
  vector<Media> medias = mediaDirectory.medias();
  std::sort( medias.begin(), medias.end(), []( const Media & a, const Media & b )->bool { return ( a.filename() < b.filename() ); } );

  for( std::shared_ptr<MediaDirectory> directory: mediaDirectory.subDirectories() )
  {
    directoryAdded( directory->path().string(), new DirectoryCollectionPath {directory->path(), mediaCollection, this} );
  }

  for( Media media : medias )
    mediaAdded( media );
}


bool DirectoryCollectionPath::hasMedia( Media &media )
{
  return media.parentDirectory() == path;
}

bool DirectoryCollectionPath::hasMediaInSubPath( Media &media )
{
  return media.parentDirectory().string().find( path.string() ) != string::npos;
}


void RootCollectionPath::render( OnDirectoryAdded directoryAdded, OnMediaAdded mediaAdded )
{
  for( string directory : settings->mediasDirectories( session ) )
  {
    if( mediaCollection->isAllowed( directory ) )
      directoryAdded( directory, new DirectoryCollectionPath {directory, mediaCollection, this} );
  }
}


string DirectoryCollectionPath::label() const
{
  return path.filename().string();
}

string RootCollectionPath::label() const
{
  return wtr( "mediacollection.root" ).toUTF8();
}



void MediaCollectionBrowser::Private::addDirectory( CollectionPath *directory )
{
  auto onClick = [ = ]( WMouseEvent )
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
  // TODO: very messy... FFMPEGMedia is of course deleted, need to create a copy...
  FFMPEGMedia *ffmpegMedia = new FFMPEGMedia {media, [=](const string &level) { return wApp->log(level); } };
  WDialog *dialog = new WDialog( wtr( "mediabrowser.admin.setposter" ) );
  auto createThumbs = new CreateThumbnails {wApp, settings, dialog};
  dialog->footer()->addWidget( WW<WPushButton>( wtr( "button.cancel" ) ).css( "btn btn-danger" ).onClick( [ = ]( WMouseEvent )
  {
    dialog->reject();
    delete ffmpegMedia;
  } ) );
  dialog->footer()->addWidget( WW<WPushButton>( wtr( "button.ok" ) ).css( "btn btn-success" ).onClick( [ = ]( WMouseEvent )
  {
    Dbo::Transaction t( *session );
    createThumbs->save( &t );
    t.commit();
    dialog->accept();
    q->reload();
    delete ffmpegMedia;
  } ) );
  dialog->show();
  dialog->resize( 500, 500 );
  auto runStep = [ = ]
  {
    Dbo::Transaction t( *session );
    createThumbs->run( ffmpegMedia, media, dialog->contents(), &t, MediaScannerStep::OverwriteIfExisting );
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
  breadcrumb->addWidget( WW<WPushButton>( wtr( "mediacollection.reload" ) ).css( "btn btn-small" )
                         .onClick( [ = ]( WMouseEvent )
  {
    Dbo::Transaction t( *session );

    for( auto path : collectionPaths )
    {
      if( path.first == ROOT_PATH_ID )
        continue;

      delete path.second;
      collectionPaths.erase( path.first );
    }

    collection->rescan( t );
    browse( collectionPaths[ROOT_PATH_ID] );
  } ) );
  list<CollectionPath *> paths;
  CollectionPath *current = currentPath;

  while( current )
  {
    paths.push_front( current );
    current = current->parent();
  }

  goToParent->setEnabled( paths.size() > 1 );

  for( CollectionPath * path : paths )
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
  for( auto collectionPath : d->collectionPaths )
  {
    delete collectionPath.second;
    d->collectionPaths.erase( collectionPath.first );
  }
}

