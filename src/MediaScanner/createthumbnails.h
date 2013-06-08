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

#ifndef CREATETHUMBNAILS_H
#define CREATETHUMBNAILS_H

#include "MediaScanner/mediascannerstep.h"

namespace Wt {
class WPushButton;
}
namespace StreamingPrivate {
  class CreateThumbnailsPrivate;
}

class Settings;
class CreateThumbnails : public MediaScannerStep, Wt::WObject
{
public:
    ~CreateThumbnails();
    CreateThumbnails(Wt::WApplication *app, Settings* settings, Wt::WObject* parent = 0);
    void run(FFMPEGMedia* ffmpegMedia, Media media, Wt::WContainerWidget* container, Wt::Dbo::Transaction *transaction, ExistingFlags onExisting = SkipIfExisting);
    virtual void save(Wt::Dbo::Transaction *transaction);
    virtual StepResult result();
    virtual Wt::Signal<> &redo();
    inline virtual std::string stepName() const {return "createThumbnails"; }
private:
  StreamingPrivate::CreateThumbnailsPrivate* const d;
};

#endif // CREATETHUMBNAILS_H