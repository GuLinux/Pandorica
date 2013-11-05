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



#ifndef SAVESUBTITLESTODATABASE_H
#define SAVESUBTITLESTODATABASE_H

#include <Wt/WObject>
#include "MediaScanner/mediascannerstep.h"
#include "utils/d_ptr.h"

class Session;
class SaveSubtitlesToDatabase :  public MediaScannerStep
{
public:
    ~SaveSubtitlesToDatabase();
    SaveSubtitlesToDatabase(const std::shared_ptr<MediaScannerSemaphore> &semaphore, Wt::WApplication* app);
    virtual void run(FFMPEGMedia* ffmpegMedia, Media media, Wt::Dbo::Transaction &transaction, ExistingFlags onExisting = SkipIfExisting);
    inline virtual std::string stepName() const {return "saveSubtitlesToDatabase"; }
    virtual void setupGui( Wt::WContainerWidget *container );
protected:
    virtual void save(Wt::Dbo::Transaction &transaction);
private:
  D_PTR;
};

#endif // SAVESUBTITLESTODATABASE_H
