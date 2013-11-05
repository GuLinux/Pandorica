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



#include "MediaScanner/mediascanner.h"
#include "private/mediascanner_p.h"
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

MediaScanner::Private::Private(MediaScanner* q, MediaCollection *mediaCollection, Session *session, Settings* settings, std::function<bool(Media&)> scanFilter)
  : q(q), mediaCollection(mediaCollection), session(session), settings(settings), scanFilter(scanFilter), app(wApp)
{
}

MediaScanner::MediaScanner(Session* session, Settings* settings, MediaCollection* mediaCollection, WObject* parent, function<bool(Media&)> scanFilter)
    : d(this, mediaCollection, session, settings, scanFilter)
{
  d->accept.connect([=](_n6){ scanFinished().emit(); });
  d->reject.connect([=](_n6){ scanFinished().emit(); });
  d->semaphore = make_shared<MediaScannerSemaphore>(
    [=]{ d->scanningMediaGuiControl(true); },
    [=]{ d->scanningMediaGuiControl(false); }
  );

  d->steps = {
    make_shared<ScanMediaInfoStep>(d->semaphore, wApp),
    make_shared<SaveSubtitlesToDatabase>(d->semaphore, wApp),
    make_shared<CreateThumbnails>(d->semaphore, wApp, settings),
  };
  
}

void MediaScanner::Private::setupGui(Wt::WContainerWidget *mainContainer, Wt::WContainerWidget *buttonsContainer)
{
  progressBar = new WProgressBar;
  progressBar->addStyleClass("pull-left");
  buttonsContainer->addWidget(progressBar);
  buttonsContainer->addWidget(buttonCancel = WW<WPushButton>(wtr("button.cancel")).css("btn btn-danger").onClick([=](WMouseEvent) {
    canceled = true;
    buttonCancel->disable();
    semaphore->needsSaving(false);
    reject.emit();
  }));
  buttonsContainer->addWidget(buttonSkip = WW<WPushButton>(wtr("button.skip")).css("btn btn-warning").onClick([=](WMouseEvent) {
    buttonSkip->disable();
    semaphore->needsSaving(false);
  }));
  buttonsContainer->addWidget(buttonNext = WW<WPushButton>(wtr("button.next")).css("btn btn-primary").setEnabled(false).onClick([=](WMouseEvent) {
    buttonNext->disable();
    canContinue = true;
  }));
  buttonsContainer->addWidget(buttonClose = WW<WPushButton>(wtr("button.close")).css("btn btn-success").onClick([=](WMouseEvent) {
    buttonClose->disable();
    accept.emit();
  }).setEnabled(false));
  mainContainer->addWidget(WW<WContainerWidget>()
    .add(progressBarTitle = WW<WText>().css("mediascannerdialog-filename"))
    .padding(6)
    .setContentAlignment(AlignCenter)
  );
  WContainerWidget* stepsContainer = new WContainerWidget;
  mainContainer->addWidget(stepsContainer);

  for(auto step: steps) {
    auto groupBox = new WGroupBox(wtr(string{"stepname."} + step->stepName() ));
    groupBox->setStyleClass("step-groupbox");
    auto container = new WContainerWidget;
    groupBox->addWidget(container);
    stepsContents[step] = {groupBox, container};
    stepsContainer->addWidget(groupBox);
    groupBox->setPadding(5);
  }
}


void MediaScanner::Private::scanningMediaGuiControl(bool enabled)
{
  guiRun(app, [=] {
    buttonNext->setEnabled(enabled);
    buttonCancel->setEnabled(enabled);
    buttonSkip->setEnabled(enabled);
    app->triggerUpdate();
  });
}


MediaScanner::~MediaScanner()
{
  wApp->log("notice") << __PRETTY_FUNCTION__;
}

Signal<>& MediaScanner::scanFinished()
{
  return d->scanFinished;
}


void MediaScanner::dialog()
{
  resize(700, 650);
  setWindowTitle(wtr("mediascanner.title"));
  setClosable(false);
  setResizable(true);

  d->accept.connect([=](_n6){ accept(); });
  d->reject.connect([=](_n6){ reject(); });

  d->setupGui(contents(), footer());
  show();
  scan();
}

void MediaScanner::scan()
{
  auto updateGuiProgress = [=] {
    d->progressBar->setValue(d->scanningProgress.progress);
    d->progressBarTitle->setText(d->scanningProgress.currentFile);
    for(auto stepContainers : d->stepsContents)
      stepContainers.second.content->clear();
    d->app->triggerUpdate();
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
    d->app->triggerUpdate();
  };
  boost::thread(boost::bind(&MediaScanner::Private::scanMedias, d.get(), updateGuiProgress, onScanFinished ));
}

void MediaScanner::Private::scanMedias(function<void()> updateGuiProgress, function<void()> onScanFinish)
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
  guiRun(app, [=]{
      progressBar->setMaximum(mediaCollection->collection().size());
      app->triggerUpdate();
  });
  vector<Media> collection;
  Utils::transform(mediaCollection->collection(), collection, [](pair<string,Media> m){ return m.second; });
  sort(begin(collection), end(collection), [](const Media &_1, const Media &_2) { return _1.fullPath() < _2.fullPath(); } );
  for(auto &media: collection) {
    if(canceled) {
      return;
    }
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
    runStepsFor(media, transaction);
  }
}


void MediaScanner::Private::runStepsFor(Media media, Dbo::Transaction& transaction)
{
  canContinue = false;

  FFMPEGMedia ffmpegMedia{media, [=](const string &level) { return app->log(level); } };
  guiRun(app, [=]{
    for(auto step: steps) {
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
  for(auto step: steps) {
    threads.push_back(boost::thread([=,&transaction,&media,&ffmpegMedia]{
      step->run(&ffmpegMedia, media, transaction);
      if(! step->needsSaving())
        guiRun(app, [=] { stepsContents[step].groupBox->hide(); app->triggerUpdate(); });
    }));
    for(auto &stepThread: threads)
      stepThread.join();
//     step->run(&ffmpegMedia, media, transaction);
  }
  while(!canContinue && semaphore->needsSaving() )
    boost::this_thread::sleep_for(boost::chrono::milliseconds{200});
  for(auto step: steps) {
    step->saveIfNeeded(transaction);
  }
}


