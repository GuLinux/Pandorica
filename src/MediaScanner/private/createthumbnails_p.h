/***********************************************************************
Copyright (c) 2013 "Marco Gulino <marco.gulino@gmail.com>"

This file is part of Pandorica: https://github.com/GuLinux/Pandorica

Pandorica is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details (included the COPYING file).

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/




#ifndef CREATETHUMBNAILSPRIVATE_H
#define CREATETHUMBNAILSPRIVATE_H
#include <sys/types.h>
#include <stdint.h>
#include <Wt/WSignal>
#include <Wt/WContainerWidget>
#include "media/media.h"
#include "MediaScanner/createthumbnails.h"
#include <mutex>

class FFMPEGMedia;
class Settings;
namespace Wt
{
  class WProgressBar;
  class WText;
  class WContainerWidget;
  class WMemoryResource;
  class WFileUpload;
}

#define IMAGE_SIZE_PREVIEW 550
#define IMAGE_SIZE_THUMB 260
#define IMAGE_SIZE_PLAYER 640

typedef std::function<void( int, std::string )> UpdateGuiProgress;
typedef std::vector<uint8_t> ImageBlob;
struct ThumbnailPosition
{
  int percent;
  std::string timing;
  static ThumbnailPosition from( int timeInSeconds );
};


class ImageUploader : public Wt::WContainerWidget
{
  public:
    ImageUploader( WContainerWidget *parent = 0 );
    Wt::Signal<ImageBlob> &previewImage()
    {
      return _previewImage;
    }
  private:
    void uploaded();
    void reset();
    Wt::WFileUpload *upload;
    Wt::Signal<ImageBlob> _previewImage;
    WContainerWidget *linkContainer;
};


class CreateThumbnails::Private
{
  public:
    Private( Wt::WApplication* app, Settings* settings, CreateThumbnails* q );
    Settings *settings;
    Wt::WApplication *app;
    void thumbnailFor( int size, int quality = 8 );
    ImageBlob resize( const ImageBlob &data, uint32_t size, uint32_t quality = 75 );
    ImageBlob fullImage;

    void createThumbnailFromMedia(const std::unique_lock< MediaScannerSemaphore >& semaphoreLock);
    void findRandomPosition();

    Media currentMedia;
    ThumbnailPosition currentPosition;
    FFMPEGMedia *currentFFMPEGMedia;
    Wt::WImage *previewImage;
    double currentTime;
    double duration;
    Wt::WMemoryResource *thumbnail = 0;
    bool redoSignalWasConnected = false;
  private:
    class CreateThumbnails *const q;
};

#endif // CREATETHUMBNAILSPRIVATE_H

