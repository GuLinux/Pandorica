/*
 * Copyright 2013 Marco Gulino <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef MEDIASCANNERDIALOGPRIVATE_H
#define MEDIASCANNERDIALOGPRIVATE_H
#include <vector>

class MediaScannerStep;
class Session;
class Settings;
namespace Wt {
class WProgressBar;
class WText;
class WApplication;
class WPushButton;
class WContainerWidget;
}

class MediaCollection;

typedef std::function<void(int,std::string)> UpdateGuiProgress;
typedef std::function<void()> OnScanFinish;

class MediaScannerDialogPrivate
{
public:
    MediaScannerDialogPrivate(MediaScannerDialog* q, MediaCollection* mediaCollection, Session* session, Settings* settings);
    virtual ~MediaScannerDialogPrivate();
    Wt::WPushButton* buttonNext;
    Wt::WPushButton* buttonClose;
    std::vector<MediaScannerStep*> steps;
    MediaCollection* mediaCollection;
    Wt::WProgressBar* progressBar;
    Wt::WText* progressBarTitle;
    Settings* settings;
    Session* session;
    Wt::WContainerWidget* stepContent;
    Wt::WPushButton* buttonRetry;
    void scanMedias(Wt::WApplication* app, UpdateGuiProgress updateGuiProgress, OnScanFinish onScanFinish);
    
private:
    class MediaScannerDialog* const q;
};

#endif // MEDIASCANNERDIALOGPRIVATE_H
