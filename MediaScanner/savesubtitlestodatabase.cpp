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

#include "savesubtitlestodatabase.h"
#include "private/savesubtitlestodatabase_p.h"
#include <ffmpegmedia.h>
#include <chrono>
#include <Wt/WContainerWidget>
#include <Wt/WText>
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include "ffmpegmedia.h"
#include "session.h"
#include <iostream>
#include <fstream>
#include "Wt-Commons/wt_helpers.h"
#include "Models/models.h"

using namespace Wt;
using namespace std;
using namespace std::chrono;
using namespace StreamingPrivate;

SaveSubtitlesToDatabasePrivate::SaveSubtitlesToDatabasePrivate(Wt::WApplication* app, SaveSubtitlesToDatabase* q)
  : app(app), q(q)
{
}
SaveSubtitlesToDatabasePrivate::~SaveSubtitlesToDatabasePrivate()
{
}

SaveSubtitlesToDatabase::~SaveSubtitlesToDatabase()
{
    delete d;

}

SaveSubtitlesToDatabase::SaveSubtitlesToDatabase(WApplication* app, WObject* parent)
    : WObject(parent), d(new SaveSubtitlesToDatabasePrivate(app, this))
{
}

void SaveSubtitlesToDatabase::run(FFMPEGMedia* ffmpegMedia, Media* media, WContainerWidget* container, Dbo::Transaction* transaction)
{
  d->result = Waiting;
  vector<FFMPEG::Stream> subtitles;
  auto allStreams = ffmpegMedia->streams();
  copy_if(allStreams.begin(), allStreams.end(), back_inserter(subtitles), [=](const FFMPEG::Stream &s) {
    return s.type == FFMPEG::Subtitles;
  });
  if(subtitles.size() == 0) {
    d->result = Skip;
    return;
  }
  int subtitlesOnDb = transaction->session().query<int>("SELECT COUNT(id) FROM media_attachment WHERE media_id = ? AND type = 'subtitles'").bind(media->uid());
  cerr << "Media " << media->filename() << ": subtitles found=" << subtitles.size() << ", on db: " << subtitlesOnDb << "\n";
  if(subtitlesOnDb == subtitles.size()) {
    d->result = Skip;
    return;
  }
  transaction->session().execute("DELETE FROM media_attachment WHERE media_id = ? AND type = 'subtitles'").bind(media->uid());
  d->subtitlesToSave.clear();
  boost::thread newThread(boost::bind(&SaveSubtitlesToDatabasePrivate::extractSubtitles, d, subtitles, media, container));
}

void SaveSubtitlesToDatabasePrivate::extractSubtitles(vector<FFMPEG::Stream> subtitles, Media *media, WContainerWidget *container)
{
  int current{0};
  for(FFMPEG::Stream subtitle: subtitles) {
    string subtitleName = subtitle.metadata["title"];
    string subtitleLanguage = subtitle.metadata["language"];
    current++;
    guiRun(app, [=] {
      container->clear();
      WString text = wtr("mediascannerdialog.subtitlesmessage").arg(current).arg(subtitles.size()).arg(subtitle.index).arg(subtitleName.empty() ? "N/A" : subtitleName).arg(subtitleLanguage);
      container->addWidget(WW<WText>(text).css("small-text"));
      wApp->triggerUpdate();
    });
    time_point<high_resolution_clock> now{high_resolution_clock::now()};
    string tempFile = (boost::format("/tmp/temp_subtitle_%d.srt") % now.time_since_epoch().count()).str();
    string cmd = (boost::format("ffmpeg -loglevel quiet -y -i \"%s\" -map 0:%d -c srt \"%s\"") % media->fullPath() % subtitle.index % tempFile).str();
    cerr << "Executing command \"" << cmd << "\"\n";
    system(cmd.c_str());
    ifstream subfile(tempFile);
    if(!subfile.is_open())
      continue; // TODO: error?
      stringstream s;
    s << subfile.rdbuf();
    subfile.close();
    std::remove(tempFile.c_str());
    vector<unsigned char> data;
    for(auto c: s.str()) {
      data.push_back(c);
    }
    MediaAttachment *subtitleAttachment = new MediaAttachment("subtitles", subtitleName, subtitleLanguage, media->uid(), "text/plain", data);
    subtitlesToSave.push_back(subtitleAttachment);
  }
  guiRun(app, [=] {
    container->clear();
    container->addWidget(WW<WText>(WString::trn("mediascannerdialog.subtitles_extracted", subtitlesToSave.size()).arg(subtitlesToSave.size())).css("small-text"));
    wApp->triggerUpdate();
  });
  result = MediaScannerStep::Done;
}


MediaScannerStep::StepResult SaveSubtitlesToDatabase::result()
{
  return d->result;
}


void SaveSubtitlesToDatabase::save(Dbo::Transaction* transaction)
{
  if(d->result != Done)
    return;
  if(d->subtitlesToSave.empty()) {
    d->result = Waiting;
    return;
  }
  for(MediaAttachment *subtitle: d->subtitlesToSave)
    transaction->session().add(subtitle);
  d->result = Waiting;
}

