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

#include "MediaScanner/mediascannerdialog.h"
#include "MediaScanner/mediascannerdialog_p.h"
#include "scanmediainfostep.h"
#include "createthumbnails.h"
#include "savesubtitlestodatabase.h"
#include <session.h>
#include <mediacollection.h>
#include "Wt-Commons/wt_helpers.h"
#include <settings.h>
#include <ffmpegmedia.h>
#include <Wt/WPushButton>
#include <Wt/WStackedWidget>
#include <Wt/WTimer>
#include <Wt/WProgressBar>
#include <thread>
#include <boost/thread.hpp>

using namespace Wt;
using namespace std;

MediaScannerDialogPrivate::MediaScannerDialogPrivate(MediaScannerDialog* q, MediaCollection *mediaCollection, Session* session, Settings* settings)
  : q(q), mediaCollection(mediaCollection), session(session), settings(settings)
{
}
MediaScannerDialogPrivate::~MediaScannerDialogPrivate()
{
}

MediaScannerDialog::MediaScannerDialog(Session* session, Settings* settings, MediaCollection* mediaCollection, WObject* parent)
    : d(new MediaScannerDialogPrivate(this, mediaCollection, session, settings))
{
  resize(700, 500);
  setWindowTitle(wtr("mediascanner.title"));
  setClosable(false);
  footer()->addWidget(d->buttonRetry = WW<WPushButton>(wtr("button.retry")).css("btn btn-warning").setEnabled(false));
  footer()->addWidget(d->buttonNext = WW<WPushButton>(wtr("button.next")).css("btn btn-primary").setEnabled(false));
  footer()->addWidget(d->buttonClose = WW<WPushButton>(wtr("close-button")).css("btn btn-success").onClick([=](WMouseEvent) { accept(); } ).setEnabled(false));

  contents()->addWidget(WW<WContainerWidget>()
    .add(d->progressBarTitle = new WText)
    .add(new WBreak)
    .add(d->progressBar = new WProgressBar)
    .padding(10)
    .setContentAlignment(AlignCenter)
  );
  
  contents()->addWidget( d->stepContent = new WContainerWidget);
  
  d->steps = {
    new ScanMediaInfoStep{d->buttonNext, session, wApp, this},
    new SaveSubtitlesToDatabase{session, wApp, this},
    new CreateThumbnails{d->buttonNext, d->buttonRetry, wApp, session, settings, this},
  };
}

MediaScannerDialog::~MediaScannerDialog()
{
  delete d;
}


void MediaScannerDialog::run()
{
  show();
  d->progressBar->setMaximum(d->mediaCollection->collection().size());
  UpdateGuiProgress updateProgressBar = [=] (int progress, string text) {
    d->progressBar->setValue(progress);
    d->progressBarTitle->setText(text);
    d->stepContent->clear();
    
    wApp->triggerUpdate();
  };
  OnScanFinish enableCloseButton = [=] {
    d->buttonClose->enable();
    d->progressBarTitle->setText("");
    d->stepContent->clear();
    wApp->triggerUpdate();
  };
  boost::thread t( boost::bind(&MediaScannerDialogPrivate::scanMedias, d, wApp, updateProgressBar, enableCloseButton) );
}

void MediaScannerDialogPrivate::scanMedias(Wt::WApplication* app, UpdateGuiProgress updateGuiProgress, OnScanFinish onScanFinish)
{
  uint current = 0;
  for(auto mediaPair: mediaCollection->collection()) {
    Media media = mediaPair.second;
    this_thread::sleep_for(chrono::milliseconds{50});
    current++;
    guiRun(app, [=] { updateGuiProgress(current, media.filename()); });
    FFMPEGMedia ffmpegMedia{media};
    for(MediaScannerStep *step: steps) {
      while(step->run(&ffmpegMedia, &media, stepContent) == MediaScannerStep::ToRedo)
        this_thread::sleep_for(chrono::milliseconds{50});
    }
  }
  this_thread::sleep_for(chrono::milliseconds{50});
  guiRun(app, [=] { onScanFinish(); });
}


