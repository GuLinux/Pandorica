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

#ifndef SCANMEDIAINFOPAGEPRIVATE_H
#define SCANMEDIAINFOPAGEPRIVATE_H
#include <string>
#include <Wt/WContainerWidget>

namespace Wt {
class WApplication;
}

class Session;

class ScanMediaInfoStepPrivate
{
public:
  ScanMediaInfoStepPrivate(ScanMediaInfoStep* q, Wt::WApplication* app);
  virtual ~ScanMediaInfoStepPrivate();
    Wt::WApplication* app;
    std::string newTitle;
    bool titleIsReady;
    MediaScannerStep::StepResult result;
    FFMPEGMedia* ffmpegMedia;
    Media* media;
    void setupGui(Wt::WContainerWidget *container, std::string titleSuggestion);
    static std::string titleHint(std::string filename);
private:
  class ScanMediaInfoStep* const q;
};

struct FindAndReplace {
  std::string regexToFind;
  std::string replacement;
  static std::vector<FindAndReplace> from(std::string filename);
};

#endif // SCANMEDIAINFOPAGEPRIVATE_H
