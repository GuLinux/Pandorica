#ifndef MEDIACOLLECTIONBROWSER_H
#define MEDIACOLLECTIONBROWSER_H

#include <Wt/WContainerWidget>
#include "mediacollection.h"

class Session;
class MediaCollectionBrowserPrivate;
class MediaCollection;
class Settings;

class MediaCollectionBrowser : public Wt::WContainerWidget
{

public:
  MediaCollectionBrowser(MediaCollection *collection, Settings *settings, Session *session, Wt::WContainerWidget* parent = 0);
  virtual ~MediaCollectionBrowser();
  Wt::Signal<Media> &queue();
  Wt::Signal<Media> &play();
  void reload();
private:
  MediaCollectionBrowserPrivate *const d;
};

#endif // MEDIACOLLECTIONBROWSER_H
