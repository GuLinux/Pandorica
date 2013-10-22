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

SaveSubtitlesToDatabase::SaveSubtitlesToDatabase( WApplication *app, WObject *parent )
  : WObject( parent ), d( app, this )
{
}

#define USE_NEW_SUBTITLES_EXTRACTOR

void SaveSubtitlesToDatabase::run( FFMPEGMedia *ffmpegMedia, Media media, WContainerWidget *container, Dbo::Transaction *transaction, MediaScannerStep::ExistingFlags onExisting )
{
  setResult( Waiting );
  vector<FFMPEG::Stream> subtitles;
  auto allStreams = ffmpegMedia->streams();
  copy_if( begin(allStreams), end(allStreams), back_inserter( subtitles ), [ = ]( const FFMPEG::Stream & s )
  {
    return s.type == FFMPEG::Subtitles;
  } );

  if( subtitles.size() == 0 )
  {
    setResult( Skip );
    return;
  }

  int subtitlesOnDb = transaction->session().query<int>( "SELECT COUNT(id) FROM media_attachment WHERE media_id = ? AND type = 'subtitles'" ).bind( media.uid() );

  if( onExisting == SkipIfExisting && subtitlesOnDb == subtitles.size() )
  {
    setResult( Skip );
    return;
  }

  d->media = media;
  transaction->session().execute( "DELETE FROM media_attachment WHERE media_id = ? AND type = 'subtitles'" ).bind( media.uid() );
  d->subtitlesToSave.clear();
#ifndef USE_NEW_SUBTITLES_EXTRACTOR
   boost::thread t( boost::bind( &SaveSubtitlesToDatabase::Private::extractSubtitles, d.get(), subtitles, container ) );
#else
  auto createProgressBar = [=]
  {
    container->clear();
    container->addWidget( WW<WText>( wtr("mediascannerdialog.subtitlesgenericmessage")).padding(20, Side::Right) );
    container->addWidget( d->progressbar = new WProgressBar() );
    d->progressbar->setMaximum( 100 );
    d->app->triggerUpdate();
    boost::thread t( boost::bind( &SaveSubtitlesToDatabase::Private::extractSubtitles, d.get(), ffmpegMedia ) );
  };
  guiRun( d->app, createProgressBar );
#endif
}


void SaveSubtitlesToDatabase::Private::extractSubtitles( FFMPEGMedia *ffmpegMedia )
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
  ffmpegMedia->extractSubtitles( [ = ] { return q->result() != MediaScannerStep::Skip && q->result() != MediaScannerStep::StepResult::Done; }, progressCallback );

  for( FFMPEG::Stream & stream : ffmpegMedia->streams() )
  {
    if( stream.type != FFMPEG::Subtitles )
      continue;

    subtitlesToSave.push_back( new MediaAttachment( "subtitles", stream.metadata["title"], stream.metadata["language"], media.uid(), "text/plain", stream.data ) );
  }

  progressCallback( 100 );
  q->setResult(MediaScannerStep::Done);

}


void SaveSubtitlesToDatabase::Private::extractSubtitles( vector< FFMPEG::Stream > subtitles, WContainerWidget *container )
{
  int current {0};
  WTime start = WTime::currentServerTime();

  for( FFMPEG::Stream subtitle : subtitles )
  {
    string subtitleName = subtitle.metadata["title"];
    string subtitleLanguage = subtitle.metadata["language"];
    current++;
    guiRun( app, [ = ]
    {
      container->clear();
      WString text = wtr( "mediascannerdialog.subtitlesmessage" ).arg( current ).arg( subtitles.size() ).arg( subtitle.index ).arg( subtitleName.empty() ? "N/A" : subtitleName ).arg( subtitleLanguage );
      container->addWidget( WW<WText>( text ).css( "small-text" ) );
      wApp->triggerUpdate();
    } );
    time_point<high_resolution_clock> now {high_resolution_clock::now()};
    string tempFile = ( boost::format( "%s/temp_subtitle_%d.srt" ) % boost::filesystem::temp_directory_path().string() % now.time_since_epoch().count() ).str();
    string cmd = ( boost::format( "ffmpeg -loglevel quiet -y -i \"%s\" -map 0:%d -c srt \"%s\"" ) % media.fullPath() % subtitle.index % tempFile ).str();
    cerr << "Executing command \"" << cmd << "\"\n";
#ifdef WIN32
    PROCESS_INFORMATION procInfo;
    STARTUPINFO info = {sizeof( info )};
    char cmdline[cmd.size() + 1];
    strcpy( cmdline, cmd.c_str() );

    if( CreateProcess( NULL, cmdline, NULL, NULL , true, 0, NULL, NULL, &info, &procInfo ) )
    {
      ::WaitForSingleObject( procInfo.hProcess, INFINITE );
      CloseHandle( procInfo.hProcess );
      CloseHandle( procInfo.hThread );
    }

#else
    system( cmd.c_str() );
#endif
    cerr << "Temp file exists? " << boost::filesystem::exists( tempFile ) << endl;
    cerr.flush();
    ifstream subfile( tempFile );

    if( !subfile.is_open() )
      continue; // TODO: error?

    stringstream s;
    s << subfile.rdbuf();
    subfile.close();
    std::remove( tempFile.c_str() );
    vector<unsigned char> data;

    for( auto c : s.str() )
    {
      data.push_back( c );
    }

    MediaAttachment *subtitleAttachment = new MediaAttachment( "subtitles", subtitleName, subtitleLanguage, media.uid(), "text/plain", data );
    subtitlesToSave.push_back( subtitleAttachment );
  }

  guiRun( app, [ = ]
  {
    container->clear();
    container->addWidget( WW<WText>( WString::trn( "mediascannerdialog.subtitles_extracted", subtitlesToSave.size() ).arg( subtitlesToSave.size() ) ).css( "small-text" ) );
    wApp->triggerUpdate();
  } );
  q->setResult(MediaScannerStep::Done);
  log( "notice" ) << "Old ffmpeg binary subtitles extraction ok! elapsed: " << start.secsTo( WTime::currentServerTime() );

}

MediaScannerStep::StepResult SaveSubtitlesToDatabase::result()
{
  boost::unique_lock<boost::mutex> lock(d->resultMutex);
  return MediaScannerStep::result();
}

void SaveSubtitlesToDatabase::setResult( MediaScannerStep::StepResult result )
{
  boost::unique_lock<boost::mutex> lock(d->resultMutex);
  MediaScannerStep::setResult( result );
}


void SaveSubtitlesToDatabase::save( Dbo::Transaction *transaction )
{
  if( result() != Done )
    return;

  if( d->subtitlesToSave.empty() )
  {
    setResult( Waiting );
    return;
  }

  transaction->session().execute( "DELETE FROM media_attachment WHERE media_id = ? AND type = 'subtitles'" ).bind( d->media.uid() );

  for( MediaAttachment * subtitle : d->subtitlesToSave )
    transaction->session().add( subtitle );

  setResult( Waiting );
}


