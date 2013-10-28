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



#ifndef CREATETHUMBNAILS_H
#define CREATETHUMBNAILS_H

#include "MediaScanner/mediascannerstep.h"
#include "utils/d_ptr.h"
namespace Wt {
class WPushButton;
}
namespace PandoricaPrivate {
  class CreateThumbnailsPrivate;
}

class Settings;
class CreateThumbnails : public MediaScannerStep, Wt::WObject
{
public:
    ~CreateThumbnails();
    CreateThumbnails(const std::shared_ptr<MediaScannerSemaphore> &semaphore, Wt::WApplication *app, Settings* settings, Wt::WObject* parent = 0);
    void run(FFMPEGMedia* ffmpegMedia, Media media, Wt::Dbo::Transaction *transaction, std::function<void(bool)> showGui, ExistingFlags onExisting = SkipIfExisting);
    virtual void save(Wt::Dbo::Transaction *transaction);
    virtual Wt::Signal<> &redo();
    inline virtual std::string stepName() const {return "createThumbnails"; }
    virtual void setupGui( Wt::WContainerWidget *container );
private:
  D_PTR;
};

#endif // CREATETHUMBNAILS_H
