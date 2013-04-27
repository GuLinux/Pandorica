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
#include "session.h"
#include "mediacollectionmanagerdialog.h"
#include <Wt/WText>
#include <Wt/WApplication>
#include <Wt/WProgressBar>
#include "Wt-Commons/wt_helpers.h"
#include "mediacollection.h"
#include "ffmpegmedia.h"
#include "mediaattachment.h"
#include "session.h"
#include "sessioninfo.h"
#include "sessiondetails.h"
#include "comment.h"
#include <boost/thread.hpp>
#include <Wt/WIOService>
#include <Wt/WTimer>
#include <thread>

using namespace std;
using namespace Wt;


MediaCollectionManagerDialog::MediaCollectionManagerDialog(Session *session, MediaCollection *mediaCollection, Wt::WObject* parent)
  : WDialog(parent), session(session), mediaCollection(mediaCollection)
{
  setWindowTitle("Scanner");
  setClosable(true);
}

MediaCollectionManagerDialog::~MediaCollectionManagerDialog()
{
}


void MediaCollectionManagerDialog::run()
{
  show();
  WProgressBar *progressBar = new WProgressBar();
  WText *text = new WText;
  contents()->addWidget(text);
  contents()->addWidget(new WBreak);
  contents()->addWidget(progressBar);
  auto customContent = new WContainerWidget();
  contents()->addWidget(customContent);
  auto startScanning = [=] (WMouseEvent) {
    progressBar->setMaximum(mediaCollection->collection().size());
    boost::thread t(boost::bind(&MediaCollectionManagerDialog::scanMediaProperties, this, wApp, [=](int progress, string file) {
      progressBar->setValue(progress);
      text->setText(file);
    }, customContent));
  };
  WTimer::singleShot(100, startScanning);
}

void MediaCollectionManagerDialog::scanMediaProperties(WApplication* app, UpdateGuiProgress updateGuiProgress, WContainerWidget* customContent)
{
  Dbo::Transaction t{*session};
  int current{0};
  WApplication::UpdateLock lock{app};
  for(auto media: mediaCollection->collection()) {
    current++;
    updateGuiProgress(current, media.second.filename());
    MediaPropertiesPtr mediaPropertiesPtr = session->find<MediaProperties>().where("media_id = ?").bind(media.first);
    if(mediaPropertiesPtr)
      continue;
    FFMPEGMedia ffmpegMedia{media.second};
    pair<int, int> resolution = ffmpegMedia.resolution();
    auto mediaProperties = new MediaProperties{media.first, "", media.second.fullPath(), ffmpegMedia.durationInSeconds(), boost::filesystem::file_size(media.second.path()), resolution.first, resolution.second};
    session->add(mediaProperties);
  }
  t.commit();
}

