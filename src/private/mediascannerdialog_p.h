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
#include <boost/concept_check.hpp>
#include <media.h>

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
namespace PandoricaPrivate{

struct StepContent {
  Wt::WGroupBox *groupBox;
  Wt::WContainerWidget *content;
};
struct ScanningProgress {
  uint16_t progress;
  std::string currentFile;
};

class MediaScannerDialogPrivate
{
public:
    MediaScannerDialogPrivate(MediaScannerDialog* q, MediaCollection* mediaCollection, Session *session, Settings* settings, std::function<bool(Media&)> scanFilter);
    virtual ~MediaScannerDialogPrivate();
    Wt::WPushButton* buttonNext;
    Wt::WPushButton* buttonClose;
    std::vector<MediaScannerStep*> steps;
    MediaCollection* mediaCollection;
    Wt::WProgressBar* progressBar;
    Wt::WText* progressBarTitle;
    Settings* settings;
    std::map<MediaScannerStep*, StepContent> stepsContents;
    Wt::WPushButton* buttonRetry;
    void scanMedias(Wt::WApplication* app, std::function<void()> updateGuiProgress, std::function<void()> onScanFinish);
    bool canContinue;
    bool canceled;
    bool skipped;
    Wt::WPushButton* buttonCancel;
    Wt::WPushButton* buttonSkip;
    Wt::Signal<> scanFinished;
    Session* session;
    std::function<bool(Media&)> scanFilter;
    ScanningProgress scanningProgress;
private:
    class MediaScannerDialog* const q;
    void runStepsFor(Media media, Wt::WApplication* app, Session& session);
};
}
#endif // MEDIASCANNERDIALOGPRIVATE_H
