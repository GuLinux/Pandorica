#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <Wt/WContainerWidget>
#include <boost/filesystem.hpp>

typedef std::pair<Wt::WWidget*,boost::filesystem::path> QueueItem;

class Playlist : public Wt::WContainerWidget
{

public:
  Playlist(Wt::WContainerWidget* parent = 0);
  virtual ~Playlist();
  void queue(boost::filesystem::path path);
  void nextItem(WWidget* itemToPlay = 0);
  Wt::Signal<boost::filesystem::path> &next();
  boost::filesystem::path first();
private:
  std::list<QueueItem> internalQueue;
  Wt::Signal<boost::filesystem::path> _next;
};

#endif // PLAYLIST_H
