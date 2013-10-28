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


#ifndef MEDIASCANNERSTEP_H
#define MEDIASCANNERSTEP_H
#include <Wt/WSignal>
#include "media/media.h"

class FFMPEGMedia;
namespace Wt {
namespace Dbo {

class Transaction;
}

class WContainerWidget;
class WApplication;
}

#define guiRun(app, f) WServer::instance()->post(app->sessionId(), f)

class MediaScannerStep
{
public:
  enum StepResult { Waiting, Done, Skip, Redo };
  enum ExistingFlags { SkipIfExisting, OverwriteIfExisting};
  virtual void setupGui(Wt::WContainerWidget *container) {}
  virtual void run(FFMPEGMedia *ffmpegMedia, Media media, Wt::Dbo::Transaction *transaction, ExistingFlags onExisting = SkipIfExisting) = 0;
  virtual StepResult result();
  virtual void setResult(StepResult result);
  virtual void save(Wt::Dbo::Transaction *transaction) = 0;
  virtual std::string stepName() const = 0;
private:
  StepResult _result;
};

#endif // MEDIASCANNERSTEP_H
