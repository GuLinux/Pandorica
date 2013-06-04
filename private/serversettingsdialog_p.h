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

#ifndef SERVERSETTINGSDIALOGPRIVATE_H
#define SERVERSETTINGSDIALOGPRIVATE_H

namespace Wt {
class WPushButton;
class WContainerWidget;
}

class MediaCollection;
class Session;
class Settings;
namespace StreamingPrivate {
class ServerSettingsDialogPrivate
{
public:
  ServerSettingsDialogPrivate(Settings* settings, Session* session, MediaCollection* mediaCollection, ServerSettingsDialog* q);
    virtual ~ServerSettingsDialogPrivate();

    Settings *settings;
    Session *session;
    MediaCollection* mediaCollection;
    Wt::WPushButton* buttonOk;
    Wt::WPushButton* buttonNext;
    Wt::WContainerWidget *selectMediaRootPage();
    Wt::WContainerWidget *selectDeployTypePage();
private:
    class ServerSettingsDialog* const q;
};
}
#endif // SERVERSETTINGSDIALOGPRIVATE_H
