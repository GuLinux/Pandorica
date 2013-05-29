#include "playlist.h"
#include "session.h"
#include <Wt/WApplication>
#include <Wt/WAnchor>
#include <Wt/WText>
#include "Wt-Commons/wt_helpers.h"
#include "private/playlist_p.h"

using namespace Wt;
using namespace std;
using namespace boost;
using namespace StreamingPrivate;
namespace fs = boost::filesystem;
using namespace WtCommons;

PlaylistPrivate::PlaylistPrivate(Session* session) : session(session)
{
}



Playlist::Playlist(Session* session, WContainerWidget* parent)
: WPanel(parent), d(new PlaylistPrivate{session})
{
  setCentralWidget(d->container = new WContainerWidget);
  setTitleBar(true);
  addStyleClass("playlist");
  titleBarWidget()->addWidget(WW<WContainerWidget>().add(new WText{"Playlist"}).css("playlist-toggle accordion-toggle"));
  titleBarWidget()->clicked().connect([=](WMouseEvent){ setCollapsed(!isCollapsed()); });
  titleBarWidget()->addStyleClass("playtlist-titlebar");
  setAnimation({WAnimation::SlideInFromTop, WAnimation::EaseOut, 500});
  setCollapsible(true);
  setCollapsed(true);
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
  next.first->parent()->addStyleClass("active");
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
  WAnchor* playlistEntry = WW<WAnchor>("", media.title(t)).css("link-hand");
  playlistEntry->addWidget(new WBreak());
  playlistEntry->clicked().connect([this,media, playlistEntry](WMouseEvent&){ nextItem(playlistEntry); });
  d->container->addWidget(WW<WContainerWidget>().add(playlistEntry));
  d->internalQueue.push_back(QueueItem(playlistEntry, media));
}