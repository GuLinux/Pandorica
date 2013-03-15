#ifndef MEDIACOLLECTIONBROWSER_H
#define MEDIACOLLECTIONBROWSER_H

#include <Wt/WContainerWidget>

class MediaCollectionBrowserPrivate;
class MediaCollection;

class MediaCollectionBrowser : public Wt::WContainerWidget
{

public:
    MediaCollectionBrowser(MediaCollection *collection, Wt::WContainerWidget* parent = 0);
    virtual ~MediaCollectionBrowser();
private:
  MediaCollectionBrowserPrivate *const d;
};

#endif // MEDIACOLLECTIONBROWSER_H
