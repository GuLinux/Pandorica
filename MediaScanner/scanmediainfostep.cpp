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

using namespace Wt;
using namespace std;
using namespace std::chrono;
namespace fs = boost::filesystem;

ScanMediaInfoStepPrivate::ScanMediaInfoStepPrivate(ScanMediaInfoStep* q, WPushButton *nextButton, Session *session, WApplication *app)
  : q(q), nextButton(nextButton), session(session), app(app)
{
}
ScanMediaInfoStepPrivate::~ScanMediaInfoStepPrivate()
{
}


ScanMediaInfoStep::ScanMediaInfoStep(WPushButton* nextButton, Session* session, WApplication* app, WObject* parent)
  : WObject(parent), d(new ScanMediaInfoStepPrivate(this, nextButton ,session, app))
{
}


std::string ScanMediaInfoStepPrivate::titleHint(std::string filename)
{
  for(auto hint: filenameToTileHints) {
    filename = boost::regex_replace(filename, boost::regex{hint.first, boost::regex::icase}, hint.second);
  }
  while(filename.find("  ") != string::npos)
    boost::replace_all(filename, "  ", " ");
  boost::algorithm::trim(filename);
  return filename;
}


MediaScannerStep::StepResult ScanMediaInfoStep::run(FFMPEGMedia* ffmpegMedia, Media* media, WContainerWidget* container)
{
  d->titleIsReady = false;
  d->newTitle = {};
  Dbo::Transaction transaction(*d->session);
  MediaPropertiesPtr mediaPropertiesPtr = d->session->find<MediaProperties>().where("media_id = ?").bind(media->uid());
  if(mediaPropertiesPtr) return Skip;
  string titleSuggestion = ffmpegMedia->metadata("title").empty() ? ScanMediaInfoStepPrivate::titleHint(media->filename()) : ffmpegMedia->metadata("title");
  guiRun(d->app, boost::bind(&ScanMediaInfoStepPrivate::setupGui, d, container, titleSuggestion));
  while(!d->titleIsReady)
    this_thread::sleep_for(milliseconds(50));
  d->nextButtonConnection.disconnect();
  pair<int, int> resolution = ffmpegMedia->resolution();
  auto mediaProperties = new MediaProperties{media->uid(), d->newTitle, media->fullPath(), ffmpegMedia->durationInSeconds(), fs::file_size(media->path()), resolution.first, resolution.second};
  d->session->add(mediaProperties);
  transaction.commit();
  return Complete;
}


void ScanMediaInfoStepPrivate::setupGui(Wt::WContainerWidget* container, std::string titleSuggestion)
{
  WLabel *label = new WLabel(wtr("mediascanner.media.title"));
  WLineEdit *editTitle = WW<WLineEdit>(titleSuggestion).css("span5");
  label->setBuddy(editTitle);
  auto accept = [=] {
    if(editTitle->text().empty()) return;
    newTitle = editTitle->text().toUTF8();
    nextButton->disable();
    titleIsReady = true;
  };
  editTitle->keyWentUp().connect([=](WKeyEvent e) {
    if(editTitle->text().empty()) {
      nextButton->disable();
      return;
    }
    if(e.key() == Wt::Key_Enter) {
      accept();
      return;
    }
    nextButton->enable();
  });
  container->addWidget(WW<WContainerWidget>().css("form-inline").add(label).add(editTitle));
  nextButton->setEnabled(!titleSuggestion.empty());
  nextButtonConnection = nextButton->clicked().connect([=](WMouseEvent) { accept(); });
  app->triggerUpdate();
}



ScanMediaInfoStep::~ScanMediaInfoStep()
{
  delete d;
}

