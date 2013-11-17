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
#include <functional>

class FFMPEGMedia;
namespace Wt {
namespace Dbo {

class Transaction;
}

class WContainerWidget;
class WApplication;
}

#define guiRun(app, ...) WServer::instance()->post(app->sessionId(), __VA_ARGS__)

class MediaScannerSemaphore : public std::enable_shared_from_this<MediaScannerSemaphore> {
public:
  MediaScannerSemaphore(std::function<void()> runOnFree, std::function<void()> runOnBusy);
  MediaScannerSemaphore(MediaScannerSemaphore& parent);
  MediaScannerSemaphore &operator=(MediaScannerSemaphore& parent);
  ~MediaScannerSemaphore();
  void needsSaving(bool saving);
  bool needsSaving();
  void lock();
  void unlock();
private:
  class Private;
  friend class Private;
  const std::unique_ptr<Private> d;
};

class MediaScannerStep
{
public:
  MediaScannerStep(const std::shared_ptr<MediaScannerSemaphore> &semaphore);
  enum ExistingFlags { SkipIfExisting, OverwriteIfExisting};
  virtual void setupGui(Wt::WContainerWidget *container) {}
  virtual void run(FFMPEGMedia *ffmpegMedia, Media media, Wt::Dbo::Transaction &transaction, ExistingFlags onExisting = SkipIfExisting) = 0;
  virtual std::string stepName() const = 0;
  void saveIfNeeded(Wt::Dbo::Transaction &transaction);
  bool needsSaving();
protected:
  virtual void save(Wt::Dbo::Transaction &transaction) = 0;
  MediaScannerSemaphore semaphore;
};

#endif // MEDIASCANNERSTEP_H
