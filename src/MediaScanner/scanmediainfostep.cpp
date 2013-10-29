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




#include "MediaScanner/scanmediainfostep.h"
#include "private/scanmediainfostep_p.h"
#include <Wt/WProgressBar>
#include <Wt/WText>
#include <Wt/WApplication>
#include <Wt/WPushButton>
#include <Wt/WLineEdit>
#include <Wt/WTimer>
#include <Wt/WLabel>
#include <Wt/WTime>
#include "Wt-Commons/wt_helpers.h"
#include <session.h>
#include "media/mediacollection.h"
#include <thread>

#include "ffmpegmedia.h"
#include <Wt/Dbo/Session>

#include "utils/utils.h"
#include "utils/d_ptr_implementation.h"

#include "Models/models.h"
#include <boost/thread.hpp>

using namespace Wt;
using namespace std;
using namespace std::chrono;
namespace fs = boost::filesystem;
using namespace WtCommons;

ScanMediaInfoStep::Private::Private( ScanMediaInfoStep *q, WApplication *app )
  : q( q ), app( app )
{
}

ScanMediaInfoStep::ScanMediaInfoStep( const shared_ptr< MediaScannerSemaphore >& semaphore, WApplication* app, WObject* parent )
  : WObject( parent ), MediaScannerStep(semaphore), d( this, app )
{
}


void ScanMediaInfoStep::run( FFMPEGMedia* ffmpegMedia, Media media, Dbo::Transaction* transaction, function<void(bool)> showGui, MediaScannerStep::ExistingFlags onExisting )
{
  d->app->log("notice") << __PRETTY_FUNCTION__;
  boost::unique_lock<MediaScannerSemaphore> semaphoreLock(semaphore);
  
  MediaPropertiesPtr mediaPropertiesPtr = media.properties( *transaction );

  if( onExisting == SkipIfExisting && mediaPropertiesPtr )
  {
    d->app->log("notice") << "skipping media";
    return;
  }
  d->app->log("notice") << "running showGui";
  showGui(true);
  semaphore.needsSaving(true);
  string titleSuggestion = ffmpegMedia->metadata( "title" ).empty() ? Utils::titleHintFromFilename( media.filename() ) : ffmpegMedia->metadata( "title" );
  d->newTitle = titleSuggestion;
  d->ffmpegMedia = ffmpegMedia;
  d->media = media;
  guiRun( d->app, [=]{ d->editTitle->setText(titleSuggestion); d->app->triggerUpdate(); } );
}


void ScanMediaInfoStep::save( Dbo::Transaction *transaction )
{
  transaction->session().execute( "DELETE FROM media_properties WHERE media_id = ?" ).bind( d->media.uid() );
  pair<int, int> resolution = d->ffmpegMedia->resolution();
  auto mediaProperties = new MediaProperties {d->media.uid(), d->newTitle, d->media.fullPath(), d->ffmpegMedia->durationInSeconds(), static_cast<int64_t>(fs::file_size( d->media.path())), resolution.first, resolution.second};
  transaction->session().add( mediaProperties );
}


void ScanMediaInfoStep::setupGui(WContainerWidget* container)
{
    WLabel *label = new WLabel( wtr( "mediascanner.media.title" ) );
    d->editTitle = WW<WLineEdit>().css( "span5" );
    d->editTitle->changed().connect( [ = ]( _n1 ) { d->newTitle = d->editTitle->text().toUTF8(); });
    label->setBuddy( d->editTitle );
    container->addWidget( WW<WContainerWidget>().css( "form-inline" ).add( label ).add( d->editTitle ) );
}




ScanMediaInfoStep::~ScanMediaInfoStep()
{
}

