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
#include "savesubtitlestodatabase_p.h"
#include <ffmpegmedia.h>
#include <chrono>
#include <Wt/WContainerWidget>
#include <Wt/WText>
#include <boost/format.hpp>
#include "ffmpegmedia.h"
#include "mediaattachment.h"
#include "session.h"
#include "sessioninfo.h"
#include "sessiondetails.h"
#include "comment.h"
#include <iostream>
#include <fstream>
#include "Wt-Commons/wt_helpers.h"

using namespace Wt;
using namespace std;
using namespace std::chrono;

SaveSubtitlesToDatabasePrivate::SaveSubtitlesToDatabasePrivate(Session* session,  Wt::WApplication* app, SaveSubtitlesToDatabase* q)
  : session(session), app(app), q(q)
{
}
SaveSubtitlesToDatabasePrivate::~SaveSubtitlesToDatabasePrivate()
{
}

SaveSubtitlesToDatabase::~SaveSubtitlesToDatabase()
{
    delete d;

}

SaveSubtitlesToDatabase::SaveSubtitlesToDatabase(Session* session, WApplication* app, Wt::WObject* parent)
    : WObject(parent), d(new SaveSubtitlesToDatabasePrivate(session, app, this))
{

}

MediaScannerStep::StepResult SaveSubtitlesToDatabase::run(FFMPEGMedia* ffmpegMedia, Media* media, Wt::WContainerWidget* container)
{
  vector<FFMPEG::Stream> subtitles;
  auto allStreams = ffmpegMedia->streams();
  copy_if(allStreams.begin(), allStreams.end(), back_inserter(subtitles), [=](const FFMPEG::Stream &s) {
    return s.type == FFMPEG::Subtitles;
  });
  if(subtitles.size() == 0) {
    return Skip;
  }
  Dbo::Transaction t(*d->session);
  int subtitlesOnDb = d->session->query<int>("SELECT COUNT(*) FROM media_attachment WHERE media_id = ? AND type = 'subtitles'").bind(media->uid());
  cerr << "Media " << media->filename() << ": subtitles found=" << subtitles.size() << ", on db: " << subtitlesOnDb << "\n";
  if(subtitlesOnDb == subtitles.size())
    return Skip;
  d->session->execute("DELETE FROM media_attachment WHERE media_id = ? AND type = 'subtitles'").bind(media->uid());
  for(FFMPEG::Stream subtitle: subtitles) {
    string subtitleName = subtitle.metadata["title"];
    string subtitleLanguage = subtitle.metadata["language"];
    guiRun(d->app, [=] {
      container->clear();
      container->addWidget(new WText{wtr("mediascannerdialog.subtitlesmessage").arg(subtitle.index).arg(subtitleName.empty() ? "N/A" : subtitleName).arg(subtitleLanguage)});
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
    d->session->add(subtitleAttachment);
  }
  t.commit();
  return Complete;
}
