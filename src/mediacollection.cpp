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

using namespace Wt;
using namespace std;
using namespace boost;
using namespace Wt::Utils;
using namespace PandoricaPrivate;

namespace fs = boost::filesystem;

MediaCollection::MediaCollection(Settings *settings, Session* session, WApplication* parent)
: WObject(parent), d(new MediaCollectionPrivate(settings, session, parent))
{
  setUserId(session->user().id());
}

void MediaCollection::rescan(Dbo::Transaction &transaction)
{
  WServer::instance()->post(d->app->sessionId(), [=] {
    d->loadingIndicator = new WOverlayLoadingIndicator();
    wApp->root()->addWidget(d->loadingIndicator->widget());
    d->loadingIndicator->widget()->show();
    wApp->triggerUpdate();
  });
  UserPtr user = transaction.session().find<User>().where("id = ?").bind(d->userId);
  d->allowedPaths = user->allowedPaths();
  d->collection.clear();
  for(fs::path p: d->settings->mediasDirectories(&transaction.session()))
    d->listDirectory(p);
  WServer::instance()->post(d->app->sessionId(), [=] {
    d->scanned.emit();
    d->loadingIndicator->widget()->hide();
    wApp->triggerUpdate();
    delete d->loadingIndicator;
  });
}

bool MediaCollectionPrivate::isAllowed(filesystem::path path)
{
    for(string p: allowedPaths) {
        if(path.string().find(p) != string::npos)
            return true;
    }
    return false;
}

bool MediaCollection::isAllowed(const filesystem::path& path) const
{
  return d->isAllowed(path);
}

void MediaCollection::setUserId(long long int userId)
{
  d->userId = userId;
}

long long MediaCollection::viewingAs() const
{
  return d->userId;
}


Media resolveMedia(fs::path path) {
  while(fs::is_symlink(path)) {
    path = fs::read_symlink(path);
  }
  if(fs::is_regular_file(path))
    return Media{path};
  return Media::invalid();
}

void MediaCollectionPrivate::listDirectory(filesystem::path path)
{
    vector<fs::directory_entry> v;
    try {
      copy(fs::recursive_directory_iterator(path, fs::symlink_option::recurse), fs::recursive_directory_iterator(), back_inserter(v));
        sort(v.begin(), v.end());
        for(fs::directory_entry entry: v) {
          Media media = resolveMedia(entry.path());
          if( media.valid() && isAllowed(entry.path())) {
            Media media{entry.path()};
            collection[media.uid()] = media;
          }
        }
    } catch(std::exception &e) {
        log("error") << "Error trying to add path " << path << ": " << e.what();
    }
}

map< string, Media > MediaCollection::collection() const
{
    return d->collection;
}

Media MediaCollection::media(string uid) const
{
  if(d->collection.count(uid)>0)
    return d->collection[uid];
  return {};
}

Signal<>& MediaCollection::scanned()
{
    return d->scanned;
}

vector< Media > MediaCollection::sortedMediasList() const
{
  vector<Media> medias;
  transform(d->collection.begin(), d->collection.end(),
            back_insert_iterator<vector<Media>>(medias), [](pair<string,Media> mediaElement) {
              return mediaElement.second;
            });
  sort(medias.begin(), medias.end(), [](const Media &first, const Media &second) { return lexicographical_compare(first.fullPath(), second.fullPath(), is_iless()); });
  return medias;
}


MediaCollection::~MediaCollection()
{
    delete d;
}

