#ifndef MEDIACOLLECTIONBROWSER_H
#define MEDIACOLLECTIONBROWSER_H

#include <Wt/WContainerWidget>
#include "mediacollection.h"

class MediaCollectionBrowserPrivate;
class MediaCollection;

class MediaCollectionBrowser : public Wt::WContainerWidget
{

public:
    MediaCollectionBrowser(MediaCollection *collection, Wt::WContainerWidget* parent = 0);
    virtual ~MediaCollectionBrowser();
    Wt::Signal<Media> &queue();
    Wt::Signal<Media> &play();
private:
  MediaCollectionBrowserPrivate *const d;
};

#endif // MEDIACOLLECTIONBROWSER_H