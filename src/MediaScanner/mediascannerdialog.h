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

#ifndef MEDIASCANNERDIALOG_H
#define MEDIASCANNERDIALOG_H

#include <Wt/WDialog>

class Settings;
class MediaCollection;
class Session;
namespace StreamingPrivate {
  class MediaScannerDialogPrivate;
}
class MediaScannerDialog : Wt::WDialog
{
public:
  MediaScannerDialog(Session* session, Settings* settings, MediaCollection* mediaCollection, Wt::WObject* parent);
    virtual ~MediaScannerDialog();
  void run();
  Wt::Signal<> &scanFinished();
private:
  StreamingPrivate::MediaScannerDialogPrivate* const d;
};

#endif // MEDIASCANNERDIALOG_H
