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

#ifndef SERVERSETTINGSDIALOG_H
#define SERVERSETTINGSDIALOG_H

#include <Wt/WDialog>

class MediaCollection;
class Session;
class Settings;
namespace StreamingPrivate {
  class ServerSettingsDialogPrivate;
}
class ServerSettingsDialog : public Wt::WDialog
{
public:
    ~ServerSettingsDialog();
    ServerSettingsDialog(Settings* settings, Session* session, MediaCollection* mediaCollection, Wt::WObject* parent = 0);
  void run();
private:
  StreamingPrivate::ServerSettingsDialogPrivate* const d;
};

#endif // SERVERSETTINGSDIALOG_H
