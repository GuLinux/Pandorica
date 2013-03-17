#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <Wt/WContainerWidget>
#include <boost/filesystem.hpp>
#include "media.h"

typedef std::pair<Wt::WWidget*,Media> QueueItem;

class Playlist : public Wt::WContainerWidget
{

public:
  Playlist(Wt::WContainerWidget* parent = 0);
  virtual ~Playlist();
  void queue(Media media);
  void nextItem(WWidget* itemToPlay = 0);
  Wt::Signal<Media> &next();
  Media first();
  void reset();
private:
  std::list<QueueItem> internalQueue;
  Wt::Signal<Media> _next;
};

#endif // PLAYLIST_H
