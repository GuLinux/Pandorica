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
#include "media/mediacollection.h"
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
#include <boost/thread.hpp>
#include <boost/chrono.hpp>

#include "utils/d_ptr_implementation.h"
#include <utils/utils.h>

using namespace Wt;
using namespace std;
using namespace WtCommons;

MediaScannerDialog::Private::Private(MediaScannerDialog* q, MediaCollection *mediaCollection, Session *session, Settings* settings, std::function<bool(Media&)> scanFilter)
  : q(q), mediaCollection(mediaCollection), session(session), settings(settings), scanFilter(scanFilter)
{
}

MediaScannerDialog::MediaScannerDialog(Session* session, Settings* settings, MediaCollection* mediaCollection, WObject* parent, function<bool(Media&)> scanFilter)
    : d(this, mediaCollection, session, settings, scanFilter)
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
    d->semaphore->needsSaving(false);
    reject();
  }));
  footer()->addWidget(d->buttonSkip = WW<WPushButton>(wtr("button.skip")).css("btn btn-warning").onClick([=](WMouseEvent) {
    d->semaphore->needsSaving(false);
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
  WApplication *app = wApp;
  d->semaphore = make_shared<MediaScannerSemaphore>([=]{
    guiRun(app, [=]{
      d->buttonNext->enable();
      d->buttonSkip->enable();
      d->buttonCancel->enable();
      app->triggerUpdate();
    });
  }, [=]{
    guiRun(app, [=]{
      d->buttonNext->disable();
      d->buttonSkip->disable();
      d->buttonCancel->disable();
      app->triggerUpdate();
    });
  });

  d->steps = {
    new ScanMediaInfoStep{d->semaphore, wApp, this},
    new SaveSubtitlesToDatabase{d->semaphore, wApp, this},
    new CreateThumbnails{d->semaphore, wApp, settings, this},
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
  wApp->log("notice") << __PRETTY_FUNCTION__;
}

Signal<>& MediaScannerDialog::scanFinished()
{
  return d->scanFinished;
}


void MediaScannerDialog::run()
{
  show();
  d->progressBar->setMaximum(d->mediaCollection->collection().size());
  auto updateGuiProgress = [=] {
    d->progressBar->setValue(d->scanningProgress.progress);
    d->progressBarTitle->setText(d->scanningProgress.currentFile);
    for(auto stepContainers : d->stepsContents)
      stepContainers.second.content->clear();
    wApp->triggerUpdate();
  };
  auto onScanFinished = [=] {
    d->buttonClose->enable();
    d->buttonNext->disable();
    d->buttonCancel->disable();
    d->buttonSkip->disable();
    for(auto stepContent: d->stepsContents)
      stepContent.second.groupBox->hide();
    d->progressBarTitle->setText("");
    for(auto stepContainers : d->stepsContents)
      stepContainers.second.content->clear();
    wApp->triggerUpdate();
  };
  boost::thread(boost::bind(&MediaScannerDialog::Private::scanMedias, d.get(), wApp, updateGuiProgress, onScanFinished ));
}

void MediaScannerDialog::Private::scanMedias(Wt::WApplication* app, function<void()> updateGuiProgress, function<void()> onScanFinish)
{
  canceled = false;
  semaphore->needsSaving(false);
  scanningProgress = {0, {}};
  Session session;
  Dbo::Transaction transaction(session);
  Scope onFinish([=,&transaction]{
    boost::this_thread::sleep_for(boost::chrono::milliseconds{500});
    transaction.commit();
    guiRun(app, [=] { updateGuiProgress(); onScanFinish(); });
  });
  mediaCollection->rescan(transaction);
  for(auto mediaPair: mediaCollection->collection()) {
    if(canceled) {
      return;
    }
    Media media = mediaPair.second;
    scanningProgress.progress++;
    scanningProgress.currentFile = media.filename();
    log("notice") << "Scanning file " << scanningProgress.progress << " of " << mediaCollection->collection().size() << ": " << scanningProgress.currentFile;
    if(!scanFilter(media)) {
      guiRun(app, updateGuiProgress);
      continue;
    }
    guiRun(app, [=] {
      buttonNext->disable();
      buttonSkip->disable();
      for(auto stepContent: stepsContents)
        stepContent.second.groupBox->hide();
      updateGuiProgress();
    });
    runStepsFor(media, app, transaction);
  }
}


void MediaScannerDialog::Private::runStepsFor(Media media, WApplication* app, Dbo::Transaction& transaction)
{
  canContinue = false;

  FFMPEGMedia ffmpegMedia{media, [=](const string &level) { return app->log(level); } };
  guiRun(app, [=]{
    for(MediaScannerStep *step: steps) {
      stepsContents[step].content->clear();
      step->setupGui(stepsContents[step].content);
      stepsContents[step].groupBox->show();
    }
    app->triggerUpdate();
  });
  Scope hideStepContents([=]{
    guiRun(app, [=] { for(auto stepContent: stepsContents) stepContent.second.groupBox->hide(); app->triggerUpdate(); });
  });
  list<boost::thread> threads;
  for(MediaScannerStep *step: steps) {
//     threads.push_back(boost::thread([=,&t,&media,&ffmpegMedia]{step->run(&ffmpegMedia, media, &t, showGui);}));
    step->run(&ffmpegMedia, media, transaction);
    if(! step->needsSaving())
      guiRun(app, [=] { stepsContents[step].groupBox->hide(); app->triggerUpdate(); });
  }
  while(!canContinue && semaphore->needsSaving() )
    boost::this_thread::sleep_for(boost::chrono::milliseconds{200});
  for(MediaScannerStep *step: steps) {
    step->saveIfNeeded(transaction);
  }
}


