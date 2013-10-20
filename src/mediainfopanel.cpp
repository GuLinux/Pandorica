/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Marco Gulino <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "mediainfopanel.h"
#include "private/mediainfopanel_p.h"
#include "utils/d_ptr_implementation.h"
#include "utils/utils.h"
#include "media/media.h"
#include "Models/models.h"
#include <Wt/Dbo/Transaction>
#include "session.h"
#include "Wt-Commons/wt_helpers.h"
#include "settings.h"
#include <Wt/WPushButton>
#include <Wt/WText>
#include <Wt/WImage>
#include <Wt/WTable>
#include <Wt/WTime>
#include <Wt/WMessageBox>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

using namespace std;
using namespace Wt;
using namespace WtCommons;

MediaInfoPanel::Private::Private( MediaInfoPanel *q ) : q( q )
{
}
MediaInfoPanel::Private::~Private()
{
}

MediaInfoPanel::MediaInfoPanel( Session *session, Settings *settings, Wt::WContainerWidget *parent )
: WContainerWidget( parent ), d(this), session( session ), settings( settings )
{
  setStyleClass( "browser-info-panel" );
  Dbo::Transaction t( *session );
  isAdmin = session->user()->isAdmin();
  reset();
}

MediaInfoPanel::~MediaInfoPanel()
{
}

void MediaInfoPanel::reset()
{
  clear();
  addWidget( WW<WContainerWidget>().setContentAlignment( AlignCenter )
             .add( WW<WText>( wtr( "infopanel.empty.title" ) ).css( "media-title" ) )
             .add( WW<WImage>( Settings::staticPath( "/icons/site-logo-big.png" ) ) )
           );
  addWidget( new WBreak );
  addWidget( WW<WText>( wtr( "infopanel.empty.message" ) ) );
  auto folderActions = createPanel( "mediabrowser.folderActions" );
  folderActions.second->addWidget( WW<WPushButton>( wtr( "mediabrowser.folderActions.playFolder" ) ).css( "btn btn-block btn-small btn-primary" ).onClick( [ = ]( WMouseEvent )
  {
    _playFolder.emit();
  } ) );
  folderActions.second->addWidget( WW<WPushButton>( wtr( "mediabrowser.folderActions.playFolderRecursive" ) ).css( "btn btn-block btn-small" ).onClick( [ = ]( WMouseEvent )
  {
    _playFolderRecursive.emit();
  } ) );
  addWidget( folderActions.first );
  wasResetted().emit();
}


void MediaInfoPanel::info( Media &media )
{
  clear();
  WContainerWidget *header = WW<WContainerWidget>().setContentAlignment( AlignCenter );
  Dbo::Transaction t( *session );
  WString title = media.title( t );
  header->addWidget( WW<WText>( title ).css( "media-title" ) );
  Dbo::ptr<MediaAttachment> previewAttachment = media.preview( t, Media::PreviewPlayer );

  if( previewAttachment )
  {
    WLink previewLink = previewAttachment->link( previewAttachment, t, header );
    WLink fullImage = previewLink;

    Dbo::ptr<MediaAttachment> fullImageAttachment = media.preview( t, Media::PreviewFull );

    if( fullImageAttachment )
      fullImage = fullImageAttachment->link( fullImageAttachment, t, header );

    WAnchor *fullImageLink = new WAnchor {fullImage, new WImage{previewLink, title}};
    fullImageLink->setTarget( Wt::AnchorTarget::TargetNewWindow );
    header->addWidget( fullImageLink );
  }
  else
  {
    auto iconType = ( media.mimetype().find( "video" ) == string::npos ) ? Settings::AudioFile : Settings::VideoFile;
    WImage *icon = new WImage { settings->icon( iconType ) };
    header->addWidget( icon );
  }

  auto mediaMediaInfoPanel = createPanel( "mediabrowser.information" );
  WTable *table = new WTable( mediaMediaInfoPanel.second );
  table->setWidth( WLength( 100, WLength::Percentage ) );
  labelValueBox( "mediabrowser.filename", media.filename(), table );
  labelValueBox( "mediabrowser.filesize", Utils::formatFileSize( fs::file_size( media.path() ) ), table );

  MediaPropertiesPtr mediaProperties = media.properties( t );

  if( mediaProperties )
  {
    labelValueBox( "mediabrowser.creation_time", mediaProperties->creationTime().toString(), table );
    labelValueBox( "mediabrowser.medialength", WTime( 0, 0, 0 ).addSecs( mediaProperties->duration() ).toString(), table );

    if( media.mimetype().find( "video" ) != string::npos && mediaProperties->width() > 0 && mediaProperties->height() > 0 )
      labelValueBox( "mediabrowser.resolution", WString( "{1}x{2}" ).arg( mediaProperties->width() ).arg( mediaProperties->height() ), table );
  }

  Ratings rating = MediaRating::ratingFor( media, t );

  if( rating.users > 0 )
  {
    WContainerWidget *avgRatingWidget = new WContainerWidget;

    for( int i = 1; i <= 5; i++ )
    {
      avgRatingWidget->addWidget( WW<WImage>( Settings::staticPath( "/icons/rating_small.png" ) ).css( rating.ratingAverage < i ? "rating-unrated" : "" ) );
    }

    labelValueBox( "mediabrowser.rating", avgRatingWidget, table );
  }

  auto actions = createPanel( "mediabrowser.actions" );
  actions.second->addWidget( WW<WPushButton>( wtr( "mediabrowser.play" ) ).css( "btn btn-block btn-small btn-primary" ).onClick( [ = ]( WMouseEvent )
  {
    _play.emit( media );
  } ) );
  actions.second->addWidget( WW<WPushButton>( wtr( "mediabrowser.queue" ) ).css( "btn btn-block btn-small" ).onClick( [ = ]( WMouseEvent )
  {
    _queue.emit( media );
  } ) );
  actions.second->addWidget( WW<WPushButton>( wtr( "mediabrowser.share" ) ).css( "btn btn-block btn-small" ).onClick( [ = ]( WMouseEvent )
  {
    Wt::Dbo::Transaction t( *session );
    auto shareMessageBox = new WMessageBox( wtr( "mediabrowser.share" ), wtr( "mediabrowser.share.dialog" ).arg( media.title( t ) ).arg( settings->shareLink( media.uid() ).url() ), NoIcon, Ok );
    shareMessageBox->button( Ok )->clicked().connect( [ = ]( WMouseEvent )
    {
      shareMessageBox->accept();
    } );
    shareMessageBox->show();
  } ) );
  addWidget( WW<WPushButton>( wtr( "button.close.info" ) ).css( "btn btn-primary btn-block hidden-desktop" )
             .onClick( [ = ]( WMouseEvent )
  {
    wasResetted().emit();
  } ) );
  addWidget( header );
  addWidget( mediaMediaInfoPanel.first );
  addWidget( actions.first );

  if( isAdmin )
  {
    auto adminActions = createPanel( "mediabrowser.admin.actions" );
    adminActions.first->addStyleClass( "visible-desktop" );
    adminActions.first->collapse();
    adminActions.second->addWidget( WW<WPushButton>( wtr( "mediabrowser.admin.settitle" ) ).css( "btn btn-block btn-small btn-primary" ).onClick( [ = ]( WMouseEvent )
    {
      setTitle().emit( media );
    } ) );
    adminActions.second->addWidget( WW<WPushButton>( wtr( "mediabrowser.admin.setposter" ) ).css( "btn btn-block btn-small btn-primary" ).onClick( [ = ]( WMouseEvent )
    {
      setPoster().emit( media );
    } ) );
    adminActions.second->addWidget( WW<WPushButton>( wtr( "mediabrowser.admin.deletepreview" ) ).css( "btn btn-block btn-small btn-danger" ).onClick( [ = ]( WMouseEvent )
    {
      deletePoster().emit( media );
    } ) );
    adminActions.second->addWidget( WW<WPushButton>( wtr( "mediabrowser.admin.deleteattachments" ) ).css( "btn btn-block btn-small btn-danger" ).onClick( [ = ]( WMouseEvent )
    {
      deleteAttachments().emit( media );
    } ) );
    addWidget( adminActions.first );
  }

  gotInfo().emit();
}


pair<WPanel *, WContainerWidget *> MediaInfoPanel::createPanel( string titleKey )
{
  WPanel *panel = new WPanel();
  panel->setTitle( wtr( titleKey ) );
  panel->setCollapsible( true );
  panel->setMargin( 10, Wt::Side::Top );
  settings->setAnimation( Settings::PanelAnimation, panel );
  setHeaderCollapsible( panel );
  WContainerWidget *container = new WContainerWidget();
  panel->setCentralWidget( container );
  return {panel, container};
}

void MediaInfoPanel::labelValueBox( string label, Wt::WString value, WTable *container )
{
  return labelValueBox( label, new WText {value}, container );
}

void MediaInfoPanel::labelValueBox( string label, WWidget *widget, WTable *container )
{
  int currentRow = container->rowCount();
  container->elementAt( currentRow, 0 )->addWidget( WW<WText>( wtr( label ) ).css( "label label-info" ) );
  container->elementAt( currentRow, 1 )->addWidget( WW<WContainerWidget>().css( "media-info-value" ).setContentAlignment( AlignRight ).add( widget ) );
}

