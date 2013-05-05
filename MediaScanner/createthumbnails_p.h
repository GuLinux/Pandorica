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

#ifndef CREATETHUMBNAILSPRIVATE_H
#define CREATETHUMBNAILSPRIVATE_H
#include <sys/types.h>
#include <stdint.h>
#include <Wt/Dbo/Transaction>
#include <media.h>

class FFMPEGMedia;
class Settings;
class Session;
namespace Wt {
class WProgressBar;
class WText;
class WContainerWidget;
class WMemoryResource;
}
typedef std::function<void(int,std::string)> UpdateGuiProgress;

struct ThumbnailPosition {
  int percent;
  std::string timing;
  static ThumbnailPosition from(int timeInSeconds);
};

class CreateThumbnailsPrivate
{
public:
  CreateThumbnailsPrivate(Wt::WApplication* app, Session* session, Settings* settings, CreateThumbnails* q);
    virtual ~CreateThumbnailsPrivate();
    Session* session;
    Settings* settings;
    Wt::WApplication* app;
    std::vector<uint8_t> thumbnailFor(int size, int quality = 8);
    
    void chooseRandomFrame(Media* media, Wt::Dbo::Transaction& t, Wt::WContainerWidget* container);
    ThumbnailPosition randomPosition(FFMPEGMedia *ffmpegMedia);
    void chooseFromVideoPlayer(const Media& media, Wt::Dbo::Transaction& t, Wt::WApplication* app);
    
    MediaScannerStep::StepResult result;
    Media* currentMedia;
    ThumbnailPosition currentPosition;
private:
    class CreateThumbnails* const q;
    double currentTime;
    double duration;
    Wt::WMemoryResource *thumbnail = 0;
};

#endif // CREATETHUMBNAILSPRIVATE_H

