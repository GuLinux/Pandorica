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



#ifndef MEDIASCANNERDIALOGPRIVATE_H
#define MEDIASCANNERDIALOGPRIVATE_H
#include <vector>
#include <Wt/WSignal>
#include "media/media.h"
#include "MediaScanner/mediascanner.h"

namespace std
{
  class condition_variable;
}

class MediaScannerSemaphore;
class User;
class Session;
class MediaScannerStep;
class Session;
class Settings;
namespace Wt {
class WProgressBar;
class WText;
class WApplication;
class WPushButton;
class WContainerWidget;
class WGroupBox;
}

class MediaCollection;
class Semaphore;
struct StepContent {
  Wt::WGroupBox *groupBox;
  Wt::WContainerWidget *content;
};
struct ScanningProgress {
  uint16_t progress;
  ::std::string currentFile;
};

class MediaScanner::Private
{
public:
    Private(MediaScanner* q, MediaCollection* mediaCollection, Session *session, Settings* settings);
    Wt::WPushButton* buttonNext;
    Wt::WPushButton* buttonClose;
    std::vector<std::shared_ptr<MediaScannerStep>> steps;
    MediaCollection* mediaCollection;
    Wt::WProgressBar* progressBar = 0;
    Wt::WText* progressBarTitle;
    Settings* settings;
    std::map<std::shared_ptr<MediaScannerStep>, StepContent> stepsContents;
    void scanMedias(std::function<void(Semaphore &)> updateGuiProgress, std::function<void(Semaphore &)> onScanFinish, std::function<bool(Media&)> scanFilter);
    bool canContinue;
    bool canceled;
    Wt::WPushButton* buttonCancel;
    Wt::WPushButton* buttonSkip;
    Wt::Signal<> scanFinished;
    Session* session;
    ScanningProgress scanningProgress;
    std::shared_ptr<MediaScannerSemaphore> semaphore;
    Wt::WApplication *app;
    void scanningMediaGuiControl(bool enabled);
    void setupGui(Wt::WContainerWidget *mainContainer, Wt::WContainerWidget *buttonsContainer);
    Wt::Signal<> accept;
    Wt::Signal<> reject;
private:
    class MediaScanner* const q;
    void runStepsFor(Media media, Wt::Dbo::Transaction& transaction);
};

#endif // MEDIASCANNERDIALOGPRIVATE_H
