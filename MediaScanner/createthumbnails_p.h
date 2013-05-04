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

class Settings;
class MediaCollection;
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
    CreateThumbnailsPrivate(Session* session, MediaCollection* mediaCollection, Settings* settings, CreateThumbnails* q);
    virtual ~CreateThumbnailsPrivate();
    Wt::WProgressBar* progressBar;
    Wt::WText* progressBarTitle;
    Wt::WContainerWidget* contentForEachMedia;
    MediaCollection* mediaCollection;
    Session* session;
    Settings* settings;
    void scanMedias(Wt::WApplication* app, UpdateGuiProgress updateGuiProgress);
    std::vector<uint8_t> thumbnailFor(const Media &media, int size, ThumbnailPosition position, int quality = 8);
    
    void saveThumbnails(std::string mediaId, const std::vector< uint8_t >& forPlayer, const std::vector< uint8_t >& forThumbnail, Wt::Dbo::Transaction& t);
    void chooseRandomFrame(const Media& media, Wt::Dbo::Transaction& t, Wt::WApplication* app);
    void chooseFromVideoPlayer(const Media& media, Wt::Dbo::Transaction& t, Wt::WApplication* app);
    enum Action {
      None, Accept, NewRandom, FromVideo
    };
private:
    class CreateThumbnails* const q;
    double currentTime;
    double duration;
    Action action;
    Wt::WMemoryResource *thumbnail = 0;
};
#endif // CREATETHUMBNAILSPRIVATE_H

