#ifndef LATESTCOMMENTSDIALOG_H
#define LATESTCOMMENTSDIALOG_H

#include <Wt/WDialog>
#include <Wt/WSignal>
#include "media.h"

class MediaCollection;
class Session;

class LatestCommentsDialog : public Wt::WDialog
{
public:
    LatestCommentsDialog(Session *session, MediaCollection *mediaCollection, Wt::WObject* parent = 0);
    virtual ~LatestCommentsDialog();
    Wt::Signal<Media> &mediaClicked();
private:
    Wt::Signal<Media> _mediaClicked;
};

#endif // LATESTCOMMENTSDIALOG_H
