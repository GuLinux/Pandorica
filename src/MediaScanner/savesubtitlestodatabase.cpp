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




#include "savesubtitlestodatabase.h"
#include "private/savesubtitlestodatabase_p.h"
#include <ffmpegmedia.h>
#include <chrono>
#include <Wt/WContainerWidget>
#include <Wt/WText>
#include <Wt/WIOService>
#include <boost/format.hpp>
#include "ffmpegmedia.h"
#include "session.h"
#include <iostream>
#include <fstream>
#include "Wt-Commons/wt_helpers.h"
#include "Models/models.h"
#include <boost/thread.hpp>

using namespace Wt;
using namespace std;
using namespace std::chrono;
using namespace StreamingPrivate;
using namespace WtCommons;

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

void SaveSubtitlesToDatabase::run(FFMPEGMedia* ffmpegMedia, Media media, WContainerWidget* container, Dbo::Transaction* transaction, MediaScannerStep::ExistingFlags onExisting)
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
  int subtitlesOnDb = transaction->session().query<int>("SELECT COUNT(id) FROM media_attachment WHERE media_id = ? AND type = 'subtitles'").bind(media.uid());
  if(onExisting == SkipIfExisting && subtitlesOnDb == subtitles.size()) {
    d->result = Skip;
    return;
  }
  d->media = media;
  transaction->session().execute("DELETE FROM media_attachment WHERE media_id = ? AND type = 'subtitles'").bind(media.uid());
  d->subtitlesToSave.clear();
  boost::thread t(boost::bind(&SaveSubtitlesToDatabasePrivate::extractSubtitles, d, subtitles, container));
}

void SaveSubtitlesToDatabasePrivate::extractSubtitles(vector< FFMPEG::Stream > subtitles, WContainerWidget* container)
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
    string cmd = (boost::format("ffmpeg -loglevel quiet -y -i \"%s\" -map 0:%d -c srt \"%s\"") % media.fullPath() % subtitle.index % tempFile).str();
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
    MediaAttachment *subtitleAttachment = new MediaAttachment("subtitles", subtitleName, subtitleLanguage, media.uid(), "text/plain", data);
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
  transaction->session().execute("DELETE FROM media_attachment WHERE media_id = ? AND type = 'subtitles'").bind(d->media.uid());
  for(MediaAttachment *subtitle: d->subtitlesToSave)
    transaction->session().add(subtitle);
  d->result = Waiting;
}

