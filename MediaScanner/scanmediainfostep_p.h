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
  ScanMediaInfoStepPrivate(ScanMediaInfoStep* q, Wt::WPushButton* nextButton, Session* session, Wt::WApplication* app);
  virtual ~ScanMediaInfoStepPrivate();
    Session* session;
    Wt::WApplication* app;
    std::string newTitle;
    bool titleIsReady;
    Wt::WPushButton* nextButton;
    boost::signals::connection nextButtonConnection;
    void setupGui(Wt::WContainerWidget *container, std::string titleSuggestion);
    static std::string titleHint(std::string filename);
private:
  class ScanMediaInfoStep* const q;
};

std::list<std::pair<std::string,std::string>> filenameToTileHints {
  // Extensions
  {"\\.mkv", ""}, {"\\.mp4", ""}, {"\\.m4v", ""}, {"\\.ogg", ""}, {"\\.mp3", ""}, {"\\.flv", ""}, {"\\.webm",""},
  // Other characters
  {"\\.", " "}, {"_", " "}, {"-", " "},
  // Resolutions and codecs
  {"\\b\\d{3,4}p\\b", "" }, {"[h|x]\\s{0,1}264", ""}, {"bluray", ""}, {"aac", ""}, {"ac3", ""}, {"dts", ""}, {"xvid", ""},
  // Dates
  {"\\b(19|20)\\d{2}\\b", "" },
  // rippers
  {"by \\w+", "" },
  // track number
  {"^\\s*\\d+", ""},
  // langs
  {"\\bita\\b", ""}, {"\\beng\\b", ""}, {"\\chd\\b", ""},  {"chs", ""},
  // everything inside brackets
  {"(\\(|\\[|\\{).*(\\)|\\]|\\})", ""},
  // various
  {"subs", ""}, {"chaps", ""}, {"chapters", ""},
  {"extended", ""}, {"repack", ""},
};


#endif // SCANMEDIAINFOPAGEPRIVATE_H
