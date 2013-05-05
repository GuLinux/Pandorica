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
  resize(700, 650);
  setWindowTitle(wtr("mediascanner.title"));
  setClosable(false);
  footer()->addWidget(d->buttonNext = WW<WPushButton>(wtr("button.next")).css("btn btn-primary").setEnabled(false).onClick([=](WMouseEvent) { d->canContinue = true; }));
  footer()->addWidget(d->buttonClose = WW<WPushButton>(wtr("close-button")).css("btn btn-success").onClick([=](WMouseEvent) { accept(); } ).setEnabled(false));

  contents()->addWidget(WW<WContainerWidget>()
    .add(d->progressBarTitle = WW<WText>().css("mediascannerdialog-filename"))
    .add(d->progressBar = new WProgressBar)
    .padding(6)
    .setContentAlignment(AlignCenter)
  );
  
  d->steps = {
    new ScanMediaInfoStep{session, wApp, this},
    new SaveSubtitlesToDatabase{session, wApp, this},
    new CreateThumbnails{wApp, session, settings, this},
  };
  
  WContainerWidget* stepsContainer = new WContainerWidget;
  contents()->addWidget(stepsContainer);

  for(auto step: d->steps) {
    auto container = new WContainerWidget;
    d->stepsContents[step] = container;
    stepsContainer->addWidget(container);
  }
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
    for(auto stepContainers : d->stepsContents)
      stepContainers.second->clear();
    wApp->triggerUpdate();
  };
  OnScanFinish enableCloseButton = [=] {
    d->buttonClose->enable();
    d->progressBarTitle->setText("");
    for(auto stepContainers : d->stepsContents)
      stepContainers.second->clear();
    wApp->triggerUpdate();
  };
  boost::thread t( boost::bind(&MediaScannerDialogPrivate::scanMedias, d, wApp, updateProgressBar, enableCloseButton) );
}

void MediaScannerDialogPrivate::scanMedias(Wt::WApplication* app, UpdateGuiProgress updateGuiProgress, OnScanFinish onScanFinish)
{
  uint current = 0;
  for(auto mediaPair: mediaCollection->collection()) {
    Media media = mediaPair.second;
    this_thread::sleep_for(chrono::milliseconds{10});
    current++;
    guiRun(app, [=] {
      buttonNext->disable();
      updateGuiProgress(current, media.filename());
    });
    runStepsFor(&media, app);
  }
  this_thread::sleep_for(chrono::milliseconds{50});
  guiRun(app, [=] { onScanFinish(); });
}


void MediaScannerDialogPrivate::runStepsFor(Media *media, WApplication* app)
{
  canContinue = false;
  FFMPEGMedia ffmpegMedia{*media};
  for(MediaScannerStep *step: steps) {
    step->run(&ffmpegMedia, media, stepsContents[step]);
  }
  while(!canContinue) {
    bool stepsAreSkipped = true;
    bool stepsAreFinished = true;
    
    for(MediaScannerStep *step: steps) {
      auto stepResult = step->result();
      stepsAreSkipped &= stepResult == MediaScannerStep::Skip;
      stepsAreFinished &= stepResult == MediaScannerStep::Skip || stepResult == MediaScannerStep::Done;
      
      if(stepResult == MediaScannerStep::Redo)
        step->run(&ffmpegMedia, media, stepsContents[step]);
    }
    canContinue |= stepsAreSkipped;
    this_thread::sleep_for(chrono::milliseconds{50});
    guiRun(app, [=] {
      buttonNext->setEnabled(stepsAreFinished);
      wApp->triggerUpdate();
    });
  }
  for(MediaScannerStep *step: steps) {
    step->save();
  }
}


