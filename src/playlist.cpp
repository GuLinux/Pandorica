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


#include "playlist.h"
#include "session.h"
#include <Wt/WApplication>
#include <Wt/WAnchor>
#include <Wt/WText>
#include "Wt-Commons/wt_helpers.h"
#include "private/playlist_p.h"
#include "settings.h"
#include <Wt/WImage>

using namespace Wt;
using namespace std;
using namespace boost;
using namespace PandoricaPrivate;
namespace fs = boost::filesystem;
using namespace WtCommons;

PlaylistPrivate::PlaylistPrivate(Session* session) : session(session)
{
}



Playlist::Playlist(Session* session, Settings* settings, WContainerWidget* parent)
: WPanel(parent), d(new PlaylistPrivate{session})
{
  setCentralWidget(d->container = new WContainerWidget);
  setTitleBar(true);
  addStyleClass("playlist");
  titleBarWidget()->addWidget(WW<WContainerWidget>().add(new WText{"Playlist"}).css("playlist-toggle accordion-toggle"));
  setHeaderCollapsible(this);
  titleBarWidget()->addStyleClass("playtlist-titlebar");
  settings->setAnimation(Settings::PlaylistAnimation, this);
  setCollapsible(true);
  collapse();
  d->container->setList(true);
  d->container->addStyleClass("nav nav-pills nav-stacked");
  d->container->setMargin(5, Side::Bottom);
}

Playlist::~Playlist()
{
  delete d;
}


Signal<Media>& Playlist::next()
{
  return d->next;
}

Media Playlist::first()
{
  return d->internalQueue.front().second;
}

void Playlist::nextItem(WWidget* itemToPlay)
{
  wApp->log("notice") << "itemToPlay==" << itemToPlay;
  if(d->internalQueue.empty()) return;

  auto itemToSkip = d->internalQueue.begin();
  while(itemToSkip->first != itemToPlay) {
    d->container->removeWidget(itemToSkip->first);
    delete itemToSkip->first;
    itemToSkip = d->internalQueue.erase(itemToSkip);
    if(!itemToPlay) break;
  }
  
  wApp->log("notice") << "outside the loop: internalQueue.size(): " << d->internalQueue.size();
  if(d->internalQueue.empty()) return;
  QueueItem next = *d->internalQueue.begin();
  next.first->addStyleClass("active");
  d->next.emit(next.second);
}

void Playlist::reset()
{
  d->container->clear();
  d->internalQueue.clear();
}



void Playlist::queue(Media media)
{
  Dbo::Transaction t(*d->session);
  WAnchor *widget = WW<WAnchor>("");
  widget->addWidget(WW<WText>(media.title(t)).css("link-hand").onClick([=](WMouseEvent&){ nextItem(widget); }));
  widget->addWidget(WW<WText>("<button class='close pull-right'>&times;</button>").onClick([=](WMouseEvent){
    d->internalQueue.erase(remove_if(d->internalQueue.begin(), d->internalQueue.end(), [=](QueueItem item) { return item.first == widget; }));
    delete widget;
  }));
  d->container->addWidget(widget);
  d->internalQueue.push_back({widget, media});
}