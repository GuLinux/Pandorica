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
#include <Wt/WIOService>
#include <thread>

using namespace Wt;
using namespace std;
using namespace PandoricaPrivate;
using namespace WtCommons;

MediaScannerDialogPrivate::MediaScannerDialogPrivate(MediaScannerDialog* q, MediaCollection *mediaCollection, Session *session, Settings* settings, std::function<bool(Media&)> scanFilter)
  : q(q), mediaCollection(mediaCollection), session(session), settings(settings), scanFilter(scanFilter)
{
}
MediaScannerDialogPrivate::~MediaScannerDialogPrivate()
{
}

MediaScannerDialog::MediaScannerDialog(Session* session, Settings* settings, MediaCollection* mediaCollection, WObject* parent, function<bool(Media&)> scanFilter)
    : d(new MediaScannerDialogPrivate(this, mediaCollection, session, settings, scanFilter))
{
  resize(700, 650);
  setWindowTitle(wtr("mediascanner.title"));
  setClosable(false);
  setResizable(true);
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
  footer()->addWidget(d->buttonClose = WW<WPushButton>(wtr("button.close")).css("btn btn-success").onClick([=](WMouseEvent) {
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
    auto groupBox = new WGroupBox(wtr(string{"stepname."} + step->stepName() ));
    groupBox->setStyleClass("step-groupbox");
    auto container = new WContainerWidget;
    groupBox->addWidget(container);
    d->stepsContents[step] = {groupBox, container};
    stepsContainer->addWidget(groupBox);
    groupBox->setPadding(5);
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


void MediaScannerDialogPrivate::updateProgress() {
    progressBar->setValue(scanningProgress.progress);
    progressBarTitle->setText(scanningProgress.currentFile);
    for(auto stepContainers : stepsContents)
      stepContainers.second.content->clear();
    wApp->triggerUpdate();
}

void MediaScannerDialog::run()
{
  show();
  d->progressBar->setMaximum(d->mediaCollection->collection().size());
  auto enableCloseButton = [=] {
    d->buttonClose->enable();
    d->buttonCancel->disable();
    d->progressBarTitle->setText("");
    for(auto stepContainers : d->stepsContents)
      stepContainers.second.content->clear();
    wApp->triggerUpdate();
  };
  WServer::instance()->ioService().post(boost::bind(&MediaScannerDialogPrivate::scanMedias, d, wApp, bind(&MediaScannerDialogPrivate::updateProgress, d), enableCloseButton ));
}

void MediaScannerDialogPrivate::scanMedias(Wt::WApplication* app, function<void()> updateGuiProgress, function<void()> onScanFinish)
{
  canceled = false;
  scanningProgress = {0, {}};
  Session session;
  Dbo::Transaction transaction(session);
  mediaCollection->rescan(transaction);
  
  for(auto mediaPair: mediaCollection->collection()) {
    if(canceled)
      return;
    Media media = mediaPair.second;
    scanningProgress.progress++;
    scanningProgress.currentFile = media.filename();
    log("notice") << "Scanning file " << scanningProgress.progress << " of " << mediaCollection->collection().size() << ": " << scanningProgress.currentFile;
    if(!scanFilter(media)) {
      continue;
    }
    guiRun(app, [=] {
      buttonNext->disable();
      buttonSkip->disable();
      for(auto stepContent: stepsContents)
        stepContent.second.groupBox->hide();
      updateGuiProgress();
    });
      runStepsFor(media, app, session);
  }
  this_thread::sleep_for(chrono::milliseconds{10});
  guiRun(app, [=] { updateGuiProgress(); onScanFinish(); });
}


void MediaScannerDialogPrivate::runStepsFor(Media media, WApplication* app, Session& session)
{
  canContinue = false;
  skipped = false;
  FFMPEGMedia ffmpegMedia{media};
  Dbo::Transaction t(session);
  for(MediaScannerStep *step: steps) {
    step->run(&ffmpegMedia, media, stepsContents[step].content, &t);
  }
  while(!canContinue && !canceled && !skipped) {
    bool stepsAreSkipped = true;
    bool stepsAreFinished = true;

    
    for(MediaScannerStep *step: steps) {
      auto stepResult = step->result();
      if(stepResult != MediaScannerStep::Skip)
        guiRun(app, [=] { stepsContents[step].groupBox->show(); });
      stepsAreSkipped &= stepResult == MediaScannerStep::Skip;
      stepsAreFinished &= stepResult == MediaScannerStep::Skip || stepResult == MediaScannerStep::Done;
      
      if(stepResult == MediaScannerStep::Redo)
        step->run(&ffmpegMedia, media, stepsContents[step].content, &t);
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


