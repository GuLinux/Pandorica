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




#include "savesubtitlestodatabase.h"
#include "private/savesubtitlestodatabase_p.h"
#include <ffmpegmedia.h>
#include <chrono>
#include <Wt/WContainerWidget>
#include <Wt/WText>
#include <Wt/WIOService>
#include <Wt/WTime>
#include <Wt/WProgressBar>
#include <boost/format.hpp>
#include "ffmpegmedia.h"
#include "session.h"
#include <iostream>
#include <fstream>
#include "Wt-Commons/wt_helpers.h"
#include "Models/models.h"
#include <boost/thread.hpp>
#include "utils/d_ptr_implementation.h"
#include <utils/utils.h>
#include <settings.h>
#ifdef WIN32
#include <windows.h>
#endif

using namespace Wt;
using namespace std;
using namespace std::chrono;
using namespace WtCommons;

SaveSubtitlesToDatabase::Private::Private( Wt::WApplication *app, SaveSubtitlesToDatabase *q )
  : app( app ), q( q )
{
}

SaveSubtitlesToDatabase::~SaveSubtitlesToDatabase()
{
}

SaveSubtitlesToDatabase::SaveSubtitlesToDatabase( const shared_ptr<MediaScannerSemaphore>& semaphore, WApplication* app, WObject* parent )
  : WObject( parent ), MediaScannerStep(semaphore), d( app, this )
{
}

void SaveSubtitlesToDatabase::run( FFMPEGMedia* ffmpegMedia, Media media, Dbo::Transaction& transaction, function<void(bool)> showGui, MediaScannerStep::ExistingFlags onExisting )
{
  auto semaphoreLock = make_shared<boost::unique_lock<MediaScannerSemaphore>>(semaphore);
  vector<FFMPEG::Stream> subtitles;
  auto allStreams = ffmpegMedia->streams();
  copy_if( begin( allStreams ), end( allStreams ), back_inserter( subtitles ), [ = ]( const FFMPEG::Stream & s )
  {
    return s.type == FFMPEG::Subtitles;
  } );

  if( subtitles.size() == 0 )
  {
    return;
  }

  int subtitlesOnDb = transaction.session().query<int>( "SELECT COUNT(id) FROM media_attachment WHERE media_id = ? AND type = 'subtitles'" ).bind( media.uid() );

  if( onExisting == SkipIfExisting && subtitlesOnDb == subtitles.size() )
  {
    return;
  }
  showGui(true);
  semaphore.needsSaving(true);

  d->media = media;
  transaction.session().execute( "DELETE FROM media_attachment WHERE media_id = ? AND type = 'subtitles'" ).bind( media.uid() );
  d->subtitlesToSave.clear();
  d->extractSubtitles(ffmpegMedia, semaphoreLock);
//   boost::thread t( boost::bind( &SaveSubtitlesToDatabase::Private::extractSubtitles, d.get(), ffmpegMedia, semaphoreLock) );
}

void SaveSubtitlesToDatabase::setupGui( WContainerWidget *container )
{
  container->addWidget( WW<WText>( wtr( "mediascannerdialog.subtitlesgenericmessage" ) ).padding( 20, Side::Right ) );
  container->addWidget( d->progressbar = new WProgressBar() );
  d->progressbar->setMaximum( 100 );
}



void SaveSubtitlesToDatabase::Private::extractSubtitles( FFMPEGMedia *ffmpegMedia, const shared_ptr<boost::unique_lock<MediaScannerSemaphore>> &semaphoreLock )
{
  auto progressCallback = [ = ]( double p )
  {
    progress = p;
    guiRun( app, [ = ]
    {
      progressbar->setValue( progress );
      app->triggerUpdate();
    } );
  };
  ffmpegMedia->extractSubtitles( [ = ] { return q->semaphore.needsSaving(); }, progressCallback );

  for( FFMPEG::Stream & stream : ffmpegMedia->streams() )
  {
    if( stream.type != FFMPEG::Subtitles )
      continue;

    subtitlesToSave.push_back( new MediaAttachment( "subtitles", stream.metadata["title"], stream.metadata["language"], media.uid(), "text/plain", stream.data ) );
  }

  progressCallback( 100 );

}



void SaveSubtitlesToDatabase::save( Dbo::Transaction& transaction )
{
  if(d->subtitlesToSave.empty()) return;

  transaction.session().execute( "DELETE FROM media_attachment WHERE media_id = ? AND type = 'subtitles'" ).bind( d->media.uid() );

  for( MediaAttachment * subtitle : d->subtitlesToSave )
    transaction.session().add( subtitle );
}


