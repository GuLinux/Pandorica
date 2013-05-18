#ifndef PLAYLIST_PRIVATE
#define PLAYLIST_PRIVATE
#include <Wt/WSignal>
#include "media.h"

namespace Wt {
class WWidget;
class WContainerWidget;
}

namespace StreamingPrivate {
  typedef std::pair<Wt::WWidget*,Media> QueueItem;
  
  class PlaylistPrivate {
  public:
    PlaylistPrivate(Session *session);
    Session *session;
    std::list<QueueItem> internalQueue;
    Wt::Signal<Media> next;
    Wt::WContainerWidget *container;
  };
}
#endif
