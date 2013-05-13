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
#include "private/mediascannerdialog_p.h"
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
#include <Wt/Dbo/backend/Postgres>
#include <Wt/WGroupBox>
#include <thread>
#include <boost/thread.hpp>

using namespace Wt;
using namespace std;
using namespace StreamingPrivate;

MediaScannerDialogPrivate::MediaScannerDialogPrivate(MediaScannerDialog* q, MediaCollection *mediaCollection, Settings* settings)
  : q(q), mediaCollection(mediaCollection), settings(settings)
{
}
MediaScannerDialogPrivate::~MediaScannerDialogPrivate()
{
}

MediaScannerDialog::MediaScannerDialog(Settings* settings, MediaCollection* mediaCollection, WObject* parent)
    : d(new MediaScannerDialogPrivate(this, mediaCollection, settings))
{
  resize(700, 650);
  setWindowTitle(wtr("mediascanner.title"));
  setClosable(false);
  d->progressBar = new WProgressBar;
  d->progressBar->addStyleClass("pull-left");
  footer()->addWidget(d->progressBar);
  footer()->addWidget(d->buttonCancel = WW<WPushButton>(wtr("button.cancel")).css("btn btn-danger").onClick([=](WMouseEvent) {
    d->canceled = true;
    reject();
  }));
  footer()->addWidget(d->buttonSkip = WW<WPushButton>(wtr("button.skip")).css("btn btn-warning").onClick([=](WMouseEvent) {
    d->skipped = true;
  }));
  footer()->addWidget(d->buttonNext = WW<WPushButton>(wtr("button.next")).css("btn btn-primary").setEnabled(false).onClick([=](WMouseEvent) { d->canContinue = true; }));
  footer()->addWidget(d->buttonClose = WW<WPushButton>(wtr("close-button")).css("btn btn-success").onClick([=](WMouseEvent) {
    accept();
  }).setEnabled(false));
  contents()->addWidget(WW<WContainerWidget>()
    .add(d->progressBarTitle = WW<WText>().css("mediascannerdialog-filename"))
    .padding(6)
    .setContentAlignment(AlignCenter)
  );
  finished().connect([=](DialogCode code, _n5) {
    scanFinished().emit();
  });
  d->steps = {
    new ScanMediaInfoStep{wApp, this},
    new SaveSubtitlesToDatabase{wApp, this},
    new CreateThumbnails{wApp, settings, this},
  };
  
  WContainerWidget* stepsContainer = new WContainerWidget;
  contents()->addWidget(stepsContainer);

  for(auto step: d->steps) {
    auto container = new WGroupBox(wtr(string{"stepname."} + step->stepName() ));
    container->setStyleClass("step-groupbox");
    d->stepsContents[step] = container;
    stepsContainer->addWidget(container);
    container->setPadding(5);
  }
}

MediaScannerDialog::~MediaScannerDialog()
{
  delete d;
}

Signal<>& MediaScannerDialog::scanFinished()
{
  return d->scanFinished;
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
    d->buttonCancel->disable();
    d->progressBarTitle->setText("");
    for(auto stepContainers : d->stepsContents)
      stepContainers.second->clear();
    wApp->triggerUpdate();
  };
  // TODO: muovere nel thread, appena MediaCollection::rescan() diventa thread safe
  d->mediaCollection->rescan();
  boost::thread t( boost::bind(&MediaScannerDialogPrivate::scanMedias, d, wApp, updateProgressBar, enableCloseButton) );
}

void MediaScannerDialogPrivate::scanMedias(Wt::WApplication* app, UpdateGuiProgress updateGuiProgress, OnScanFinish onScanFinish)
{
  canceled = false;
  uint current = 0;
  Session session;
  for(auto mediaPair: mediaCollection->collection()) {
    if(canceled)
      return;
    Media media = mediaPair.second;
    current++;
    guiRun(app, [=] {
      buttonNext->disable();
      buttonSkip->disable();
      for(auto stepContent: stepsContents)
        stepContent.second->hide();
      updateGuiProgress(current, media.filename());
    });
    runStepsFor(media, app, session);
  }
  this_thread::sleep_for(chrono::milliseconds{10});
  guiRun(app, [=] { onScanFinish(); });
}


void MediaScannerDialogPrivate::runStepsFor(Media media, WApplication* app, Session& session)
{
  canContinue = false;
  skipped = false;
  FFMPEGMedia ffmpegMedia{media};
  Dbo::Transaction t(session);
  for(MediaScannerStep *step: steps) {
    step->run(&ffmpegMedia, media, stepsContents[step], &t);
  }
  while(!canContinue && !canceled && !skipped) {
    bool stepsAreSkipped = true;
    bool stepsAreFinished = true;

    
    for(MediaScannerStep *step: steps) {
      auto stepResult = step->result();
      if(stepResult != MediaScannerStep::Skip)
        guiRun(app, [=] { stepsContents[step]->show(); });
      stepsAreSkipped &= stepResult == MediaScannerStep::Skip;
      stepsAreFinished &= stepResult == MediaScannerStep::Skip || stepResult == MediaScannerStep::Done;
      
      if(stepResult == MediaScannerStep::Redo)
        step->run(&ffmpegMedia, media, stepsContents[step], &t);
    }
    canContinue |= stepsAreSkipped;
    guiRun(app, [=] {
      buttonSkip->enable();
      buttonNext->setEnabled(stepsAreFinished && ! stepsAreSkipped);
      wApp->triggerUpdate();
    });
    if(!canContinue && !canceled && !skipped)
      this_thread::sleep_for(chrono::milliseconds{50});
  }
  if(canceled)
    return;
  if(!skipped) {
    for(MediaScannerStep *step: steps) {
      step->save(&t);
    }
    t.commit();
  }
}


