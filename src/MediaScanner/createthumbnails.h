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

namespace Wt {
class WPushButton;
}
namespace StreamingPrivate {
  class CreateThumbnailsPrivate;
}

class Settings;
class CreateThumbnails : public MediaScannerStep, Wt::WObject
{
public:
    ~CreateThumbnails();
    CreateThumbnails(Wt::WApplication *app, Settings* settings, Wt::WObject* parent = 0);
    void run(FFMPEGMedia* ffmpegMedia, Media media, Wt::WContainerWidget* container, Wt::Dbo::Transaction *transaction, ExistingFlags onExisting = SkipIfExisting);
    virtual void save(Wt::Dbo::Transaction *transaction);
    virtual StepResult result();
    virtual Wt::Signal<> &redo();
    inline virtual std::string stepName() const {return "createThumbnails"; }
private:
  StreamingPrivate::CreateThumbnailsPrivate* const d;
};

#endif // CREATETHUMBNAILS_H
