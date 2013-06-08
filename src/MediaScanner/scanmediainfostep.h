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



#ifndef SCANMEDIAINFOPAGE_H
#define SCANMEDIAINFOPAGE_H

#include <Wt/WContainerWidget>
#include "mediascannerstep.h"

class Session;
class MediaCollection;
namespace StreamingPrivate {
  class ScanMediaInfoStepPrivate;
}
class ScanMediaInfoStep : public MediaScannerStep, Wt::WObject
{
public:
  ScanMediaInfoStep(Wt::WApplication *app, Wt::WObject *parent = 0);
  virtual ~ScanMediaInfoStep();
  virtual void run(FFMPEGMedia* ffmpegMedia, Media media, Wt::WContainerWidget *container, Wt::Dbo::Transaction *transaction, ExistingFlags onExisting = SkipIfExisting);
  virtual StepResult result();
  virtual void save(Wt::Dbo::Transaction *transaction);
  inline virtual std::string stepName() const {return "scanMediaInfoStep"; }
private:
  StreamingPrivate::ScanMediaInfoStepPrivate* const d;
};

#endif // SCANMEDIAINFOPAGE_H
