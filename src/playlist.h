#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <Wt/WPanel>
#include <boost/filesystem.hpp>
#include "media.h"

namespace StreamingPrivate {
  class PlaylistPrivate;
}

class Playlist : public Wt::WPanel
{

public:
  Playlist(Session *session, Wt::WContainerWidget* parent = 0);
  virtual ~Playlist();
  void queue(Media media);
  void nextItem(WWidget* itemToPlay = 0);
  Wt::Signal<Media> &next();
  Media first();
  void reset();
private:
  StreamingPrivate::PlaylistPrivate *const d;
};

#endif // PLAYLIST_H
