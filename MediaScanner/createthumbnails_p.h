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

class Settings;
class MediaCollection;
class Session;
namespace Wt {
class WProgressBar;
class WText;
class WContainerWidget;
}
typedef std::function<void(int,std::string)> UpdateGuiProgress;

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
    
private:
    class CreateThumbnails* const q;
    double currentTime;
    double duration;
};

#endif // CREATETHUMBNAILSPRIVATE_H