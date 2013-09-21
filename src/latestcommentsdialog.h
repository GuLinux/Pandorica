/***********************************************************************
Copyright (c) 2013 "Marco Gulino <marco.gulino@gmail.com>"

This file is part of Pandorica: https://github.com/GuLinux/Pandorica

Pandorica is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details (included the COPYING file).

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/


#ifndef LATESTCOMMENTSDIALOG_H
#define LATESTCOMMENTSDIALOG_H

#include <Wt/WDialog>
#include <Wt/WSignal>
#include "media/media.h"

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
