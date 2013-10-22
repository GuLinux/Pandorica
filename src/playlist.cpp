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
#include "utils/d_ptr_implementation.h"

using namespace Wt;
using namespace std;
namespace fs = boost::filesystem;
using namespace WtCommons;

Playlist::Private::Private(Playlist* playlist, Session* session) : q(playlist), session(session)
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
    auto element = std::find(begin(queue), end(queue), queueItem);
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
    queue.erase(std::remove(begin(queue), end(queue), queueItem));
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
: WPanel(parent), d(this, session)
{
  setCentralWidget(d->container = new WContainerWidget);
  setTitleBar(true);
  addStyleClass("playlist");
  setHeaderCollapsible(this);
  titleBarWidget()->addStyleClass("playtlist-titlebar");
  setCollapsible(false);

  d->container->hide();
  d->container->setList(true);
  d->container->addStyleClass("nav nav-pills nav-stacked");
  d->container->setMargin(5, Side::Bottom);
  WText *showHideButtonText = WW<WText>();
  
  d->setPlaylistVisible = [=](bool visible) {
    if(!visible) {
      showHideButtonText->setText(wtr("playlist.show"));
      settings->animateHide(Settings::PlaylistAnimation, d->container);
    }
    else {
      showHideButtonText->setText(wtr("playlist.hide"));
      settings->animateShow(Settings::PlaylistAnimation, d->container);
    }
  };
  
  d->playSignal.connect([=](PlaylistItem*,_n5){ d->setPlaylistVisible(false); });

  WContainerWidget *firstGroup = WW<WContainerWidget>().css("btn-group");
  firstGroup->addWidget(WW<WAnchor>().css("btn btn-mini")
    .setImage(WW<WImage>(Settings::staticPath("/icons/actions/playlist.png")).setMargin(5, Side::Right))
    .add(showHideButtonText)
    .onClick([=](WMouseEvent){
      d->setPlaylistVisible(!d->container->isVisible());
    }));
  firstGroup->addWidget(WW<WAnchor>().css("btn btn-mini")
    .setImage(WW<WImage>(Settings::staticPath("/icons/actions/playlist.clear.png")).setMargin(5, Side::Right))
    .setText(wtr("playlist.clear"))
    .onClick([=](WMouseEvent){
    for(auto item: d->internalQueue)
      delete item;
    d->internalQueue.clear();
  }));

  WContainerWidget *secondGroup = WW<WContainerWidget>().css("btn-group");
  WWidget *prev = WW<WAnchor>(secondGroup).css("btn btn-mini")
    .add(new WImage{Settings::staticPath("/icons/actions/previous.png")}).onClick(boost::bind(&Playlist::previous, this));
  WWidget *next = WW<WAnchor>(secondGroup).css("btn btn-mini")
    .add(new WImage{Settings::staticPath("/icons/actions/next.png")}).onClick(boost::bind(&Playlist::next, this));
  WContainerWidget *playlistButtonsContainer = WW<WContainerWidget>(titleBarWidget()).css("btn-toolbar")
  .add(firstGroup).add(secondGroup);
  d->setPlaylistVisible(false);
}

Playlist::~Playlist()
{
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
  d->playlistIncrement(Playlist::Private::Forwards);
}

void Playlist::previous()
{
  d->playlistIncrement(Playlist::Private::Backwards);
}

void Playlist::Private::playlistIncrement(Playlist::Private::Direction direction)
{
  if(internalQueue.empty()) return;
  if(0 == count_if(begin(internalQueue), end(internalQueue), [=](QueueItem *i) { return i->isCurrent(); } )) {
    playSignal.emit(internalQueue.front());
    return;
  }
  
  auto playingItem = find_if(begin(internalQueue), end(internalQueue), [=](QueueItem *i){ return i->isCurrent();});
  if(playingItem == internalQueue.end()) {
    return;
  }
  q->play( direction == Forwards ? *++playingItem : *--playingItem);
}

void Playlist::play(PlaylistItem* itemToPlay)
{
  if(itemToPlay && std::find(begin(d->internalQueue), end(d->internalQueue), itemToPlay) != d->internalQueue.end()) {
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