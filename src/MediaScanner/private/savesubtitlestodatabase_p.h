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



#ifndef SAVESUBTITLESTODATABASEPRIVATE_H
#define SAVESUBTITLESTODATABASEPRIVATE_H
#include <vector>
#include <ffmpegmedia.h>
#include "MediaScanner/savesubtitlestodatabase.h"

class MediaAttachment;
class Session;
namespace Wt {
  class WApplication;
class WProgressBar;
};

class SaveSubtitlesToDatabase::Private
{
public:
  Private(Wt::WApplication* app, SaveSubtitlesToDatabase* q);
    void extractSubtitles(std::vector< FFMPEG::Stream > subtitles, Wt::WContainerWidget* container);
    void extractSubtitles(FFMPEGMedia *ffmpegMedia);
    Wt::WApplication* app;
    MediaScannerStep::StepResult result;
    std::vector<MediaAttachment*> subtitlesToSave;
    Media media;

    double progress = 0;
    Wt::WProgressBar *progressbar;
private:
    class SaveSubtitlesToDatabase* const q;
};


#endif // SAVESUBTITLESTODATABASEPRIVATE_H
