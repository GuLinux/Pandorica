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
#include <Wt/WPushButton>

using namespace Wt;
using namespace std;
using namespace boost;
using namespace PandoricaPrivate;
namespace fs = boost::filesystem;
using namespace WtCommons;

PlaylistPrivate::PlaylistPrivate(Playlist* playlist, Session* session) : q(playlist), session(session)
{
}

  
QueueItem::QueueItem(Media media, std::list< QueueItem* >& queue, WContainerWidget* container, Session* session, WContainerWidget* parent)
  : WContainerWidget(parent), PlaylistItem(media)
{
  QueueItem *queueItem = this;
  Dbo::Transaction t(*session);
  WAnchor *anchor = new WAnchor{this};
  anchor->addWidget(WW<WText>(media.title(t)).css("link-hand").onClick([=](WMouseEvent&){ playSignal.emit(this); }));
  WContainerWidget *actionsContainer = WW<WContainerWidget>(anchor).css("pull-right");
  auto fixButtons = [=,&queue] {
    for(QueueItem *item: queue) {
      item->upButton->setHidden(item == queue.front());
      item->downButton->setHidden(item == queue.back());
    }
  };
  
  auto moveElement = [=,&queue](int direction) {
    auto element = std::find(queue.begin(), queue.end(), queueItem);
    auto nextElement = element;
    direction>0 ? nextElement++ : nextElement--;
    swap(*nextElement, *element);
    int index = container->indexOf(queueItem);
    container->removeWidget(queueItem);
    container->insertWidget(index + direction, queueItem);
    fixButtons();
  };
  
  actionsContainer->addWidget(upButton = WW<WImage>(Settings::staticPath("/icons/actions/up.png"))
    .css("link-hand").onClick([=,&queue](WMouseEvent){
    if(queue.front() == queueItem) return;
    moveElement(-1);
  }));
  actionsContainer->addWidget(downButton = WW<WImage>(Settings::staticPath("/icons/actions/down.png"))
    .css("link-hand").onClick([=,&queue](WMouseEvent){
    if(queue.back() == queueItem) return;
    moveElement(+1);
  }));
  actionsContainer->addWidget(removeButton = WW<WImage>(Settings::staticPath("/icons/actions/delete.png"))
    .css("link-hand").onClick([=,&queue](WMouseEvent){
    queue.erase(std::remove(queue.begin(), queue.end(), queueItem));
    delete queueItem;
    fixButtons();
  }));
  upButton->setHiddenKeepsGeometry(true);
  downButton->setHiddenKeepsGeometry(true);
  container->addWidget(this);
  queue.push_back(this);
  fixButtons();
}


  
bool QueueItem::isCurrent()
{
  return hasStyleClass("active");
}

void QueueItem::setActive(bool active)
{
  if(active)
    addStyleClass("active");
  else
    removeStyleClass("active");
}


Playlist::Playlist(Session* session, Settings* settings, WContainerWidget* parent)
: WPanel(parent), d(new PlaylistPrivate{this, session})
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
  d->container->addWidget(WW<WPushButton>("Clear Playlist").css("btn btn-block btn-warning").onClick([=](WMouseEvent){
    for(auto item: d->internalQueue)
      delete item;
    d->internalQueue.clear();
  }));
}

Playlist::~Playlist()
{
  delete d;
}


Wt::Signal<PlaylistItem*>& Playlist::play()
{
  return d->playSignal;
}

void Playlist::playing(PlaylistItem* currentItem)
{
  for(auto item: d->internalQueue) {
    item->setActive(currentItem == item);
  }
}

void Playlist::next()
{
  d->playlistIncrement(PlaylistPrivate::Forwards);
}

void Playlist::previous()
{
  d->playlistIncrement(PlaylistPrivate::Backwards);
}

void PlaylistPrivate::playlistIncrement(PlaylistPrivate::Direction direction)
{
  if(0 == count_if(internalQueue.begin(), internalQueue.end(), [=](QueueItem *i) { return i->isCurrent(); } )) {
    playSignal.emit(internalQueue.front());
    return;
  }
  
  auto playingItem = find_if(internalQueue.begin(), internalQueue.end(), [=](QueueItem *i){ return i->isCurrent();});
  if(playingItem == internalQueue.end()) {
    return;
  }
  q->play( direction == Forwards ? *++playingItem : *--playingItem);
}

void Playlist::play(PlaylistItem* itemToPlay)
{
  if(itemToPlay && std::find(d->internalQueue.begin(), d->internalQueue.end(), itemToPlay) != d->internalQueue.end()) {
    d->playSignal.emit(itemToPlay);
  }
}

void Playlist::reset()
{
  d->container->clear();
  d->internalQueue.clear();
}


PlaylistItem* Playlist::queue(Media media)
{
  auto item = new QueueItem(media, d->internalQueue, d->container, d->session);
  item->play().connect(this, &Playlist::play);
  return item;
}