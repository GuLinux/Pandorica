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
#include <Wt/WToolBar>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <mutex>

#include "utils/d_ptr_implementation.h"
#include <utils/utils.h>
#include <utils/semaphore.h>
#include <condition_variable>

using namespace Wt;
using namespace std;
using namespace WtCommons;

MediaScanner::Private::Private(MediaScanner* q, MediaCollection *mediaCollection, Session *session, Settings* settings)
  : q(q), mediaCollection(mediaCollection), session(session), settings(settings), app(wApp)
{
}

MediaScanner::MediaScanner(Session* session, Settings* settings, MediaCollection* mediaCollection)
    : d(this, mediaCollection, session, settings)
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
  mainContainer->clear();
  buttonsContainer->clear();
  progressBar = WW<WProgressBar>().css("pull-left").setMargin(5, Side::Left | Side::Right);
  buttonsContainer->addWidget(progressBar);
  WToolBar *buttonsToolbar = new WToolBar;
  buttonsContainer->addWidget(buttonsToolbar);
  buttonsToolbar->addButton(buttonCancel = WW<WPushButton>(wtr("button.cancel")).css("btn btn-danger").onClick([=](WMouseEvent) {
    canceled = true;
    buttonCancel->disable();
    semaphore->needsSaving(false);
    reject.emit();
  }));
  buttonsToolbar->addButton(buttonSkip = WW<WPushButton>(wtr("button.skip")).css("btn btn-warning").onClick([=](WMouseEvent) {
    buttonSkip->disable();
    semaphore->needsSaving(false);
  }));
  buttonsToolbar->addButton(buttonNext = WW<WPushButton>(wtr("button.next")).css("btn btn-primary").setEnabled(false).onClick([=](WMouseEvent) {
    buttonNext->disable();
    canContinue = true;
  }));
  buttonsToolbar->addButton(buttonClose = WW<WPushButton>(wtr("button.close")).css("btn btn-success").onClick([=](WMouseEvent) {
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

void MediaScanner::setup( WContainerWidget *content, WContainerWidget *footer )
{
  d->setupGui(content, footer);
}


void MediaScanner::dialog()
{
  WDialog *dialog = new WDialog();
  dialog->resize(700, 650);
  dialog->setWindowTitle(wtr("mediascanner.title"));
  dialog->setClosable(false);
  dialog->setResizable(true);

  d->accept.connect([=](_n6){ dialog->accept(); });
  d->reject.connect([=](_n6){ dialog->reject(); });

  d->setupGui(dialog->contents(), dialog->footer());
  dialog->show();
}


void MediaScanner::scan(function<bool(Media&)> scanFilter)
{
  auto updateGuiProgress = [=] (Semaphore &s) {
    d->progressBar->setValue(d->scanningProgress.progress);
    d->progressBarTitle->setText(WString::fromUTF8(d->scanningProgress.currentFile));
    for(auto stepContainers : d->stepsContents)
      stepContainers.second.content->clear();
    d->app->triggerUpdate();
    s.release();
  };
  auto onScanFinished = [=] (Semaphore &s) {
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
    s.release();
  };
  d->scanningProgress = {0, {}};
  Semaphore s(1);
  updateGuiProgress(s);
  s.wait();
  WServer::instance()->ioService().post(boost::bind(&MediaScanner::Private::scanMedias, d.get(), updateGuiProgress, onScanFinished, scanFilter ));
}

void MediaScanner::Private::scanMedias(function<void(Semaphore &)> updateGuiProgress, function<void(Semaphore &)> onScanFinish, function<bool(Media &)> scanFilter)
{
  canceled = false;
  semaphore->needsSaving(false);
  Session session;
  Scope onFinish([=]{
    Semaphore s(2);
    boost::this_thread::sleep_for(boost::chrono::milliseconds{500});
    guiRun(app, [=,&s] { 
      updateGuiProgress(s);
      onScanFinish(s);
    });
    s.wait();
  });
  {
    Semaphore s(1);
    guiRun(app, [=,&s] {
      mediaCollection->rescan([=,&s]{ s.release(); });
    });
    s.wait();
  }
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
    Dbo::Transaction transaction(session);
    scanningProgress.progress++;
    scanningProgress.currentFile = media.filename();
    log("notice") << "Scanning file " << scanningProgress.progress << " of " << mediaCollection->collection().size() << ": " << scanningProgress.currentFile;
    if(!scanFilter(media)) {
      Semaphore s(1);
      log("notice") << "Skipping media: " << media.fullPath();
      guiRun(app, [=,&s]{ updateGuiProgress(s); });
      s.wait();
      continue;
    }
    Semaphore s(1);
    guiRun(app, [=,&s] {
      buttonNext->disable();
      buttonSkip->disable();
      for(auto stepContent: stepsContents)
        stepContent.second.groupBox->hide();
      updateGuiProgress(s);
    });
    s.wait();
    runStepsFor(media, transaction);
  }
}


void MediaScanner::Private::runStepsFor(Media media, Dbo::Transaction& transaction)
{
  canContinue = false;

  FFMPEGMedia ffmpegMedia{media, [=](const string &level) { return WServer::instance()->log(level); } };
  Semaphore s(1);
  guiRun(app, [=, &s]{
    for(auto step: steps) {
      stepsContents[step].content->clear();
      step->setupGui(stepsContents[step].content);
      stepsContents[step].groupBox->show();
    }
    app->triggerUpdate();
    s.release();
  });
  s.wait();
  Scope hideStepContents([=]{
    Semaphore s(1);
    guiRun(app, [=,&s] { for(auto stepContent: stepsContents) stepContent.second.groupBox->hide(); app->triggerUpdate(); s.release(); });
    s.wait();
  });
  list<boost::thread> threads;
  
  for(auto step: steps) {
    threads.push_back(boost::thread([=,&transaction,&media,&ffmpegMedia]{
      step->run(&ffmpegMedia, media, transaction);
      if(! step->needsSaving()) {
	Semaphore s(1);
        guiRun(app, [=,&s] { stepsContents[step].groupBox->hide(); app->triggerUpdate(); s.release(); });
	s.wait();
      }
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


