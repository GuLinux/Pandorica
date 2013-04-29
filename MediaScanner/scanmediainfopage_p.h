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

#ifndef SCANMEDIAINFOPAGEPRIVATE_H
#define SCANMEDIAINFOPAGEPRIVATE_H
#include <string>
#include <Wt/WContainerWidget>

class MediaCollection;
class Session;
namespace Wt {
class WProgressBar;
class WPushButton;
class WLineEdit;
}

typedef std::function<void(int,std::string)> UpdateGuiProgress;

class EditMediaTitle : public Wt::WContainerWidget {
public:
  EditMediaTitle(WContainerWidget *parent = 0);
  Wt::WPushButton *okButton;
  Wt::WLineEdit *editTitle;
};

class ScanMediaInfoPagePrivate
{
public:
    ScanMediaInfoPagePrivate(Session* session, MediaCollection* mediaCollection, ScanMediaInfoPage* q);
    virtual ~ScanMediaInfoPagePrivate();
    Wt::WContainerWidget* contentForEachMedia;
    Session* session;
    MediaCollection* mediaCollection;
    Wt::WProgressBar* progressBar;
    Wt::WText* progressBarTitle;
    std::string newTitle;
    void scanMediaProperties(Wt::WApplication* app, UpdateGuiProgress updateGuiProgress);
    void editTitleWidgets(std::string suggestedTitle);
    
private:
    class ScanMediaInfoPage* const q;
    bool titleIsReady;
    EditMediaTitle* editMediaTitle;
    int secsRemaining;
};


#endif // SCANMEDIAINFOPAGEPRIVATE_H
