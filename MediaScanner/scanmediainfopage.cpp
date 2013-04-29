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

#include "MediaScanner/scanmediainfopage.h"
#include "MediaScanner/scanmediainfopage_p.h"
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

#include "ffmpegmedia.h"
#include "mediaattachment.h"
#include "session.h"
#include "sessioninfo.h"
#include "sessiondetails.h"
#include "comment.h"

using namespace Wt;
using namespace std;

EditMediaTitle::EditMediaTitle(Wt::WContainerWidget* parent): WContainerWidget(parent)
{
  setPadding(10);
  WLabel *label = new WLabel(wtr("mediascanner.media.title"));
  addWidget(editTitle = WW<WLineEdit>().css("span5"));
  label->setBuddy(editTitle);
  addWidget(okButton = WW<WPushButton>("OK").css("btn btn-primary"));
  okButton->setMinimumSize(80, WLength::Auto);
  setStyleClass("form-inline");
}


ScanMediaInfoPagePrivate::ScanMediaInfoPagePrivate(Session *session, MediaCollection *mediaCollection, ScanMediaInfoPage* q)
  : session(session), mediaCollection(mediaCollection), q(q)
{
}
ScanMediaInfoPagePrivate::~ScanMediaInfoPagePrivate()
{
}

ScanMediaInfoPage::ScanMediaInfoPage(Session* session, MediaCollection* mediaCollection, WContainerWidget* parent)
  : MediaScannerPage(parent), d(new ScanMediaInfoPagePrivate(session, mediaCollection, this))
{
  d->progressBar = new WProgressBar();
  d->progressBarTitle = new WText;
  addWidget(WW<WContainerWidget>().add(d->progressBarTitle).add(new WBreak).add(d->progressBar).setContentAlignment(AlignmentFlag::AlignCenter));
  d->contentForEachMedia = new WContainerWidget();
  addWidget(d->contentForEachMedia);
}

void ScanMediaInfoPage::run()
{
  d->progressBar->setMaximum(d->mediaCollection->collection().size());
  boost::thread t(boost::bind(&ScanMediaInfoPagePrivate::scanMediaProperties, d, wApp, [=](int progress, string file) {
    d->progressBar->setValue(progress);
    d->progressBarTitle->setText(file);
    d->contentForEachMedia->clear();
    wApp->triggerUpdate();
  }));
}



list<pair<string,string>> filenameToTileHints {
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

string titleHint(string filename) {
  for(auto hint: filenameToTileHints) {
    filename = boost::regex_replace(filename, boost::regex{hint.first, boost::regex::icase}, hint.second);
  }
  while(filename.find("  ") != string::npos)
    boost::replace_all(filename, "  ", " ");
  boost::algorithm::trim(filename);
  return filename;
}

#define guiRun(f) WServer::instance()->post(app->sessionId(), f)
void ScanMediaInfoPagePrivate::scanMediaProperties(Wt::WApplication* app, UpdateGuiProgress updateGuiProgress)
{
  int current{0};
  for(auto media: mediaCollection->collection()) {
    Dbo::Transaction t{*session};
    guiRun(boost::bind(updateGuiProgress, ++current, media.second.filename()));
    MediaPropertiesPtr mediaPropertiesPtr = session->find<MediaProperties>().where("media_id = ?").bind(media.first);
    if(mediaPropertiesPtr)
      continue;
    titleIsReady = false;
    FFMPEGMedia ffmpegMedia{media.second};
    string title = ffmpegMedia.metadata("title").empty() ? titleHint(media.second.filename()) : ffmpegMedia.metadata("title");
    guiRun([=] {
      updateGuiProgress(current, media.second.filename());
      editTitleWidgets(title);
      wApp->triggerUpdate();
    });
    
    while(!titleIsReady) {
      boost::this_thread::sleep(boost::posix_time::millisec(100));
    }
    pair<int, int> resolution = ffmpegMedia.resolution();
    auto mediaProperties = new MediaProperties{media.first, newTitle, media.second.fullPath(), ffmpegMedia.durationInSeconds(), boost::filesystem::file_size(media.second.path()), resolution.first, resolution.second};
    session->add(mediaProperties);
    t.commit();
  }
  guiRun([=] {
    contentForEachMedia->clear();
    updateGuiProgress(current, "");
    q->finished().emit();
    wApp->triggerUpdate();
  });
}


void ScanMediaInfoPagePrivate::editTitleWidgets(std::string suggestedTitle)
{
  contentForEachMedia->clear();
  contentForEachMedia->addWidget(editMediaTitle = new EditMediaTitle());
  editMediaTitle->editTitle->setText(suggestedTitle);
  secsRemaining = 10;
  editMediaTitle->okButton->setText(WString("OK ({1})").arg(secsRemaining));
  
  WTimer *timer = new WTimer(editMediaTitle);
  timer->setInterval(1000);
  auto okClicked = [=] {
    timer->stop();
    newTitle = editMediaTitle->editTitle->text().toUTF8();
    editMediaTitle->okButton->disable();
    titleIsReady = true;
  };
  timer->timeout().connect([=](WMouseEvent) {
    secsRemaining--;
    if(secsRemaining > 0)
      editMediaTitle->okButton->setText(WString("OK ({1})").arg(secsRemaining));
    else
      okClicked();
  });
  editMediaTitle->okButton->clicked().connect([=](WMouseEvent) {
    okClicked();
  });
  
  auto suspendTimer = [=] {
    timer->stop();
    editMediaTitle->okButton->setText("OK");
  };
  
  editMediaTitle->editTitle->clicked().connect([=](WMouseEvent) { suspendTimer(); });
  editMediaTitle->editTitle->focussed().connect([=](_n1) { suspendTimer(); });
  editMediaTitle->editTitle->keyWentUp().connect([=](WKeyEvent e) { 
    if(e.key() == Wt::Key_Enter)
      okClicked();
    else
      suspendTimer();
  });
  
  timer->start();
  wApp->triggerUpdate(); 
}
ScanMediaInfoPage::~ScanMediaInfoPage()
{
  delete d;
}
