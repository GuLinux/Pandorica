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

#ifndef MEDIACOLLECTIONMANAGERDIALOG_H
#define MEDIACOLLECTIONMANAGERDIALOG_H

#include <Wt/WDialog>
#include <boost/filesystem.hpp>

class MediaCollection;
class Session;
typedef std::function<void(int,std::string)> UpdateGuiProgress;

class EditMediaTitle;

class MediaCollectionManagerDialog : public Wt::WDialog
{
public:
  MediaCollectionManagerDialog(Session *session, MediaCollection *mediaCollection, Wt::WObject* parent);
  virtual ~MediaCollectionManagerDialog();
  void run();
private:
  void scanMediaProperties(Wt::WApplication* app, UpdateGuiProgress updateGuiProgress);
  void editTitleWidgets(std::string suggestedTitle);
  Session* session;
  MediaCollection* mediaCollection;
  Wt::WContainerWidget *customContent;
  EditMediaTitle *editMediaTitle = 0;
  int secsRemaining = -1;
    std::string title;
};

#endif // MEDIACOLLECTIONMANAGERDIALOG_H
