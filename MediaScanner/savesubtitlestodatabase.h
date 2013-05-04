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

#ifndef SAVESUBTITLESTODATABASE_H
#define SAVESUBTITLESTODATABASE_H

#include <Wt/WObject>
#include "MediaScanner/mediascannerstep.h"

class Session;
class SaveSubtitlesToDatabase :  Wt::WObject, public MediaScannerStep
{
public:
    ~SaveSubtitlesToDatabase();
    SaveSubtitlesToDatabase(Session* session, Wt::WApplication* app, Wt::WObject* parent);
    virtual StepResult run(FFMPEGMedia* ffmpegMedia, Media* media, Wt::WContainerWidget* container);
private:
    class SaveSubtitlesToDatabasePrivate* const d;
};

#endif // SAVESUBTITLESTODATABASE_H
