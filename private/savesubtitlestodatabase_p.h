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

#ifndef SAVESUBTITLESTODATABASEPRIVATE_H
#define SAVESUBTITLESTODATABASEPRIVATE_H
#include <vector>
#include <ffmpegmedia.h>

class MediaAttachment;
class Session;
namespace Wt {
  class WApplication;
};

namespace StreamingPrivate {
class SaveSubtitlesToDatabasePrivate
{
public:
  SaveSubtitlesToDatabasePrivate(Wt::WApplication* app, SaveSubtitlesToDatabase* q);
    virtual ~SaveSubtitlesToDatabasePrivate();
    void extractSubtitles(std::vector< FFMPEG::Stream > subtitles, Wt::WContainerWidget* container);
    Wt::WApplication* app;
    MediaScannerStep::StepResult result;
    std::vector<MediaAttachment*> subtitlesToSave;
    Media media;

private:
    class SaveSubtitlesToDatabase* const q;
};
}

#endif // SAVESUBTITLESTODATABASEPRIVATE_H
