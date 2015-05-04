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


#include "mediacollection.h"
#include "session.h"
#include <Wt/Utils>
#include <Wt/WApplication>
#include <Wt/WServer>
#include <Wt/WOverlayLoadingIndicator>
#include <thread>
#include "private/mediacollection_p.h"
#include "Models/models.h"
#include "settings.h"
#include <boost/algorithm/string.hpp>
#include "utils/d_ptr_implementation.h"
#include <boost/thread.hpp>
#include "utils/stacktrace.h"
using namespace Wt;
using namespace std;
using namespace Wt::Utils;

namespace fs = boost::filesystem;

MediaCollection::MediaCollection( Settings *settings, Session *session, WApplication *parent )
  : WObject( parent ), d(settings, session, parent )
{
}

void MediaCollection::rescan( const function< void() >& onFinish )
{
  log("notice") << __PRETTY_FUNCTION__ ;
  d->scanning.emit();
  boost::thread([=] {
    unique_lock<mutex> lock(d->rescanMutex);
    Session threadSession;
    Dbo::Transaction transaction(threadSession);
    UserPtr user = transaction.session().find<User>().where( "id = ?" ).bind( d->userId );
    d->allowedPaths = user->allowedPaths();
    d->collection.clear();
    d->mediaDirectories.clear();
    
    for( fs::path p : d->settings->mediasDirectories() ) {
      shared_ptr<MediaDirectory> rootDirectory(new MediaDirectory(p));
      d->mediaDirectories.push_back(rootDirectory);
      d->listDirectory( p, rootDirectory );
    }
    size_t size = d->collection.size();
    log("notice") << __PRETTY_FUNCTION__ << ": found " << d->collection.size() << " media files.";
      WServer::instance()->post( d->app->sessionId(), [=] {
      scanned().emit();
      onFinish();
      d->app->triggerUpdate();
    } );
  });
}

Signal< NoClass >& MediaCollection::scanning() const
{
  return d->scanning;
}


Signal<>& MediaCollection::scanned() const
{
  return d->scanned;
}


vector< shared_ptr< MediaDirectory > > MediaCollection::rootDirectories() const
{
  return d->mediaDirectories;
}

shared_ptr< MediaDirectory > MediaCollection::find( const string &directoryPath ) const
{
  for(auto rootDir: d->mediaDirectories) {
    auto findSubDirectory = d->findInSubDirectories(rootDir, directoryPath );
    if(findSubDirectory)
      return findSubDirectory;
  }
  return shared_ptr<MediaDirectory>();
}

MediaCollection::Private::MediaDirectoryPtr MediaCollection::Private::findInSubDirectories( const shared_ptr< MediaDirectory > &directory, const string &path )
{
  if(directory->relativePath() == path )
    return directory;
  for(auto dir: directory->subDirectories()) {
    auto found = findInSubDirectories(dir, path );
    if(found)
      return found;
  }
  return MediaDirectoryPtr();
}



bool MediaCollection::Private::isAllowed( fs::path path )
{
  for( string p : allowedPaths )
  {
    if( path.string().find( p ) != string::npos )
      return true;
  }

  return false;
}

bool MediaCollection::isAllowed( const fs::path &path ) const
{
  return d->isAllowed( path );
}

void MediaCollection::setUserId( long long int userId )
{
  d->userId = userId;
}

long long MediaCollection::viewingAs() const
{
  return d->userId;
}


Media resolveMedia( fs::path path )
{
  while( fs::is_symlink( path ) )
  {
    path = fs::read_symlink( path );
  }

  if( fs::is_regular_file( path ) )
    return Media {path};

  return Media::invalid();
}

void MediaCollection::Private::listDirectory( boost::filesystem::path path, shared_ptr< MediaDirectory > rootDirectory )
{
  vector<fs::directory_entry> v;

  try
  {
    fs::recursive_directory_iterator it( path, fs::symlink_option::recurse );
    while(it != fs::recursive_directory_iterator()) {
      if(it.level() > 20)
        it.pop();
      try {
        v.push_back(*it++);
      } catch(std::exception &e) {
        log("notice") << "Error scanning for path " << *it  << ": " << e.what();
        it.pop();
      }
    }
    //copy( fs::recursive_directory_iterator( path, fs::symlink_option::recurse, ec ), fs::recursive_directory_iterator(), back_inserter( v ) );
//     sort( v.begin(), v.end() ); // TODO: not needed?

    for( fs::directory_entry entry : v )
    {
      Media media = resolveMedia( entry.path() );

      if( media.valid() && isAllowed( entry.path() ) )
      {
        Media media {entry.path()};
        collection[media.uid()] = media;
        rootDirectory->add(media);
      }
    }
  }
  catch
    ( std::exception &e )
  {
    log( "error" ) << "Error trying to add path " << path << ": " << e.what();
  }
}

map< string, Media > MediaCollection::collection() const
{
  return d->collection;
}

Media MediaCollection::media( string uid ) const
{
  if( d->collection.count( uid ) > 0 )
    return d->collection[uid];

  return {};
}

vector< Media > MediaCollection::sortedMediasList() const
{
  vector<Media> medias;
  transform( begin(d->collection), end(d->collection), back_inserter( medias ), []( pair<string, Media> mediaElement )
  {
    return mediaElement.second;
  } );
  sort( begin(medias), end(medias), []( const Media & first, const Media & second )
  {
    return lexicographical_compare( first.fullPath(), second.fullPath(), boost::is_iless() );
  } );
  return medias;
}


MediaCollection::~MediaCollection()
{
}

