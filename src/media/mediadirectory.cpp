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

#include "mediadirectory.h"
#include "private/mediadirectory_p.h"
#include "utils/d_ptr_implementation.h"

using namespace std;

MediaDirectory::Private::Private( MediaDirectory *q, const boost::filesystem::path &path ) : q( q ), path( path )
{
}

MediaDirectory::MediaDirectory( const boost::filesystem::path &path ) : d( this, path )
{
}

bool MediaDirectory::operator==( const MediaDirectory &other ) const
{
  return d->path == other.d->path;
}



MediaDirectory::~MediaDirectory()
{
}

void MediaDirectory::add( const Media &media )
{
  if( media.parentDirectory() == d->path )
  {
    d->medias.push_back( media );
    return;
  }

  auto currentDirectory = media.parentDirectory();
  list<boost::filesystem::path> directories;

  while( currentDirectory != boost::filesystem::path() )
  {
    if( currentDirectory == d->path ) {
      d->addTree( media, directories );
      return;
    }

    directories.push_front( currentDirectory );
    currentDirectory = currentDirectory.parent_path();
  }
}

void MediaDirectory::Private::addTree( const Media &media, const list<boost::filesystem::path> &directories )
{
  boost::filesystem::path subdirectoryPath = directories.front();
  if(subdirectories.count(subdirectoryPath) == 0)
    subdirectories[subdirectoryPath] = shared_ptr<MediaDirectory>{new MediaDirectory{subdirectoryPath}};
  shared_ptr<MediaDirectory> subDirectory = subdirectories[subdirectoryPath];
  subDirectory->add(media);
}

vector< Media > MediaDirectory::allMedias() const
{
  vector<Media> _allMedias;
  copy(d->medias.begin(), d->medias.end(), back_insert_iterator<vector<Media>>(_allMedias));
  for(auto subDirectory: d->subdirectories) {
    auto subDirectoryMedias = subDirectory.second->allMedias();
    copy(subDirectoryMedias.begin(), subDirectoryMedias.end(), back_insert_iterator<vector<Media>>(_allMedias));
  }
  return _allMedias;
}

vector< Media > MediaDirectory::medias() const
{
  return d->medias;
}

vector< shared_ptr< MediaDirectory > > MediaDirectory::subDirectories() const
{
  vector<std::shared_ptr<MediaDirectory>> _subDirectories;
  for(auto s: d->subdirectories) {
    _subDirectories.push_back(s.second);
  }
  return _subDirectories;
}

boost::filesystem::path MediaDirectory::path() const
{
  return d->path;
}





ostream &operator<<( ostream &os, const MediaDirectory &md )
{
  os << "MediaDirectory[" << md.path() << "]";
  return os;
}
