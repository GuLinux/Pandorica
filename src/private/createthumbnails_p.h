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
#include <Wt/WSignal>
#include <Wt/WContainerWidget>
#include <Magick++/Blob.h>
#include <media.h>

namespace Magick {
class Blob;
}

class FFMPEGMedia;
class Settings;
namespace Wt {
class WProgressBar;
class WText;
class WContainerWidget;
class WMemoryResource;
class WFileUpload;
}

#define IMAGE_SIZE_PREVIEW 550
#define IMAGE_SIZE_THUMB 260
#define IMAGE_SIZE_PLAYER 640

namespace StreamingPrivate {
typedef std::function<void(int,std::string)> UpdateGuiProgress;

struct ThumbnailPosition {
  int percent;
  std::string timing;
  static ThumbnailPosition from(int timeInSeconds);
};


class ImageUploader : public Wt::WContainerWidget {
public:
  ImageUploader(WContainerWidget* parent = 0);
  Wt::Signal<Magick::Blob> &previewImage() { return _previewImage; }
private:
  void uploaded();
  void reset();
  Wt::WFileUpload *upload;
  Wt::Signal<Magick::Blob> _previewImage;
  WContainerWidget* linkContainer;
};


class CreateThumbnailsPrivate
{
public:
  CreateThumbnailsPrivate(Wt::WApplication* app, Settings* settings, CreateThumbnails* q);
    virtual ~CreateThumbnailsPrivate();
    Settings* settings;
    Wt::WApplication* app;
    void thumbnailFor(int size, int quality = 8);
    Magick::Blob resize(Magick::Blob blob, int size, uint quality = 75);
    Magick::Blob fullImage;
    
    void addImageChooser(Wt::WContainerWidget* container);
    void chooseRandomFrame(Wt::WContainerWidget* container);
    ThumbnailPosition randomPosition(FFMPEGMedia *ffmpegMedia);
    
    MediaScannerStep::StepResult result;
    Media currentMedia;
    ThumbnailPosition currentPosition;
    FFMPEGMedia* currentFFMPEGMedia;
    Wt::Signal<> redo;
private:
    class CreateThumbnails* const q;
    double currentTime;
    double duration;
    Wt::WMemoryResource *thumbnail = 0;
};
}

#endif // CREATETHUMBNAILSPRIVATE_H

