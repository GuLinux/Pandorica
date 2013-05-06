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

#include "MediaScanner/scanmediainfostep.h"
#include "MediaScanner/scanmediainfostep_p.h"
#include <Wt/WProgressBar>
#include <Wt/WText>
#include <Wt/WApplication>
#include <Wt/WPushButton>
#include <Wt/WLineEdit>
#include <Wt/WTimer>
#include <Wt/WLabel>
#include <Wt/WTime>
#include <boost/thread.hpp>
#include "Wt-Commons/wt_helpers.h"
#include <session.h>
#include <mediacollection.h>
#include <boost/regex.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <thread>

#include "ffmpegmedia.h"
#include "mediaattachment.h"
#include "session.h"
#include "sessioninfo.h"
#include "sessiondetails.h"
#include "comment.h"
#include <fstream>
#include <Wt/Json/Parser>
#include <Wt/Json/Object>
#include <Wt/Json/Array>

using namespace Wt;
using namespace std;
using namespace std::chrono;
namespace fs = boost::filesystem;

ScanMediaInfoStepPrivate::ScanMediaInfoStepPrivate(ScanMediaInfoStep* q, Session *session, WApplication *app)
  : q(q), session(session), app(app)
{
}
ScanMediaInfoStepPrivate::~ScanMediaInfoStepPrivate()
{
}


ScanMediaInfoStep::ScanMediaInfoStep(Session* session, WApplication* app, WObject* parent)
  : WObject(parent), d(new ScanMediaInfoStepPrivate(this, session, app))
{
}


vector< FindAndReplace > FindAndReplace::from(string filename)
{
  
  ifstream subfile(filename);
  if(!subfile.is_open()) {
    WServer::instance()->log("notice") << "JSON Find/Replacement file " << filename << " missing, returning empty array";
    return {};
  }
  stringstream json;
  vector<FindAndReplace> parsedVector;
  json << subfile.rdbuf();
  subfile.close();
  try {
    Json::Value parsed;
    Json::parse(json.str(), parsed);
    Json::Array parsedArray = parsed.orIfNull(Json::Array{});
    for(Json::Object value: parsedArray) {
      parsedVector.push_back({value.get("regex_to_find").toString(), value.get("replacement").toString() });
    }
    return parsedVector;
  } catch(Json::ParseError error) {
    WServer::instance()->log("notice") << "Error parsing " << filename << ": " << error.what();
    return {};
  }
}


std::string ScanMediaInfoStepPrivate::titleHint(std::string filename)
{
  for(FindAndReplace hint: FindAndReplace::from("title_from_filename_replacements.json")) {
    try {
      filename = boost::regex_replace(filename, boost::regex{hint.regexToFind, boost::regex::icase}, hint.replacement);
    } catch(runtime_error e) {
      WServer::instance()->log("notice") << "exception parsing regex '" << hint.regexToFind << "': " << e.what();
    }
  }
  while(filename.find("  ") != string::npos)
    boost::replace_all(filename, "  ", " ");
  boost::algorithm::trim(filename);
  return filename;
}


void ScanMediaInfoStep::run(FFMPEGMedia* ffmpegMedia, Media* media, WContainerWidget* container)
{
  d->result = Waiting;
  Dbo::Transaction transaction(*d->session);
  MediaPropertiesPtr mediaPropertiesPtr = d->session->find<MediaProperties>().where("media_id = ?").bind(media->uid());
  if(mediaPropertiesPtr) {
    d->result = Skip;
    return;
  }
  string titleSuggestion = ffmpegMedia->metadata("title").empty() ? ScanMediaInfoStepPrivate::titleHint(media->filename()) : ffmpegMedia->metadata("title");
  d->newTitle = titleSuggestion;
  d->ffmpegMedia = ffmpegMedia;
  d->media = media;
  guiRun(d->app, boost::bind(&ScanMediaInfoStepPrivate::setupGui, d, container, titleSuggestion));
  d->result = Done;
}
MediaScannerStep::StepResult ScanMediaInfoStep::result()
{
  return d->result;
}

void ScanMediaInfoStep::save()
{
  if(d->result != Done)
    return;
  Dbo::Transaction transaction(*d->session);
  pair<int, int> resolution = d->ffmpegMedia->resolution();
  auto mediaProperties = new MediaProperties{d->media->uid(), d->newTitle, d->media->fullPath(), d->ffmpegMedia->durationInSeconds(), fs::file_size(d->media->path()), resolution.first, resolution.second};
  d->session->add(mediaProperties);
  transaction.commit();
  d->result = Waiting;
}



void ScanMediaInfoStepPrivate::setupGui(Wt::WContainerWidget* container, std::string titleSuggestion)
{
  WLabel *label = new WLabel(wtr("mediascanner.media.title"));
  WLineEdit *editTitle = WW<WLineEdit>(titleSuggestion).css("span5");
  editTitle->changed().connect([=](_n1) {
    newTitle = editTitle->text().toUTF8();
  });
  label->setBuddy(editTitle);
  container->addWidget(WW<WContainerWidget>().css("form-inline").add(label).add(editTitle));
  app->triggerUpdate();
}



ScanMediaInfoStep::~ScanMediaInfoStep()
{
  delete d;
}

