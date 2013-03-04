#include "playlist.h"
#include <Wt/WApplication>
#include <Wt/WAnchor>

using namespace Wt;
using namespace std;
using namespace boost;
namespace fs = boost::filesystem;


Playlist::Playlist(Wt::WContainerWidget* parent): WContainerWidget(parent)
{

}

Playlist::~Playlist()
{

}


Signal< filesystem::path >& Playlist::next()
{
  return _next;
}

filesystem::path Playlist::first()
{
  return internalQueue.front().second;
}

void Playlist::nextItem(WWidget* itemToPlay)
{
  wApp->log("notice") << "itemToPlay==" << itemToPlay;
  if(internalQueue.empty()) return;

  auto itemToSkip = internalQueue.begin();
  while(itemToSkip->first != itemToPlay) {
    wApp->log("notice") << "skippedItem==" << itemToSkip->first << " [" << itemToSkip->second << "]";
    removeWidget(itemToSkip->first);
    delete itemToSkip->first;
    itemToSkip = internalQueue.erase(itemToSkip);
    if(!itemToPlay) break;
  }
  
  wApp->log("notice") << "outside the loop: internalQueue.size(): " << internalQueue.size();
  if(internalQueue.empty()) return;
  QueueItem next = *internalQueue.begin();
  wApp->log("notice") << "Next item: " << next.first << " [" << next.second << "]";
  _next.emit(next.second);
}


void Playlist::queue(filesystem::path path)
{
  WAnchor* playlistEntry = new WAnchor("javascript:false", path.filename().string());
  playlistEntry->addWidget(new WBreak());
  playlistEntry->clicked().connect([this,path, playlistEntry](WMouseEvent&){ nextItem(playlistEntry); });
  addWidget(playlistEntry);
  internalQueue.push_back(QueueItem(playlistEntry, path));
}