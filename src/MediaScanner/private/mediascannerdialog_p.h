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
#include "MediaScanner/mediascannerdialog.h"

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

struct StepContent {
  Wt::WGroupBox *groupBox;
  Wt::WContainerWidget *content;
};
struct ScanningProgress {
  uint16_t progress;
  std::string currentFile;
};

class MediaScannerDialog::Private
{
public:
    Private(MediaScannerDialog* q, MediaCollection* mediaCollection, Session *session, Settings* settings, std::function<bool(Media&)> scanFilter);
    Wt::WPushButton* buttonNext;
    Wt::WPushButton* buttonClose;
    std::vector<MediaScannerStep*> steps;
    MediaCollection* mediaCollection;
    Wt::WProgressBar* progressBar = 0;
    Wt::WText* progressBarTitle;
    Settings* settings;
    std::map<MediaScannerStep*, StepContent> stepsContents;
    void scanMedias(std::function<void()> updateGuiProgress, std::function<void()> onScanFinish);
    bool canContinue;
    bool canceled;
    Wt::WPushButton* buttonCancel;
    Wt::WPushButton* buttonSkip;
    Wt::Signal<> scanFinished;
    Session* session;
    std::function<bool(Media&)> scanFilter;
    ScanningProgress scanningProgress;
    std::shared_ptr<MediaScannerSemaphore> semaphore;
    Wt::WApplication *app;
    void scanningMediaGuiControl(bool enabled);
    void setupGui(Wt::WContainerWidget *mainContainer, Wt::WContainerWidget *buttonsContainer);
    Wt::Signal<> accept;
    Wt::Signal<> reject;
private:
    class MediaScannerDialog* const q;
    void runStepsFor(Media media, Wt::Dbo::Transaction& transaction);
};

#endif // MEDIASCANNERDIALOGPRIVATE_H
