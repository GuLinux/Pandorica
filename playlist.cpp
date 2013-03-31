#include "playlist.h"
#include "session.h"
#include <Wt/WApplication>
#include <Wt/WAnchor>

using namespace Wt;
using namespace std;
using namespace boost;
namespace fs = boost::filesystem;


Playlist::Playlist(Session* session, WContainerWidget* parent): WContainerWidget(parent), session(session)
{

}

Playlist::~Playlist()
{

}


Signal< Media >& Playlist::next()
{
  return _next;
}

Media Playlist::first()
{
  return internalQueue.front().second;
}

void Playlist::nextItem(WWidget* itemToPlay)
{
  wApp->log("notice") << "itemToPlay==" << itemToPlay;
  if(internalQueue.empty()) return;

  auto itemToSkip = internalQueue.begin();
  while(itemToSkip->first != itemToPlay) {
    removeWidget(itemToSkip->first);
    delete itemToSkip->first;
    itemToSkip = internalQueue.erase(itemToSkip);
    if(!itemToPlay) break;
  }
  
  wApp->log("notice") << "outside the loop: internalQueue.size(): " << internalQueue.size();
  if(internalQueue.empty()) return;
  QueueItem next = *internalQueue.begin();
  _next.emit(next.second);
}

void Playlist::reset()
{
  clear();
  internalQueue.clear();
}



void Playlist::queue(Media media)
{
  WAnchor* playlistEntry = new WAnchor("javascript:false", media.title(session));
  playlistEntry->addWidget(new WBreak());
  playlistEntry->clicked().connect([this,media, playlistEntry](WMouseEvent&){ nextItem(playlistEntry); });
  addWidget(playlistEntry);
  internalQueue.push_back(QueueItem(playlistEntry, media));
}