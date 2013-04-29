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
#include "scanmediainfopage.h"
#include "createthumbnails.h"
#include <session.h>
#include <mediacollection.h>
#include "Wt-Commons/wt_helpers.h"
#include <settings.h>
#include <Wt/WPushButton>
#include <Wt/WStackedWidget>
#include <Wt/WTimer>

using namespace Wt;
using namespace std;

MediaScannerDialogPrivate::MediaScannerDialogPrivate(MediaScannerDialog* q) : q(q)
{
}
MediaScannerDialogPrivate::~MediaScannerDialogPrivate()
{
}

class FinalPage : public MediaScannerPage {
public:
  FinalPage(WContainerWidget* parent = 0);
  virtual void run();
};

FinalPage::FinalPage(WContainerWidget* parent): MediaScannerPage(parent)
{
  addWidget(new WText("hello!"));
}

void FinalPage::run()
{
  WTimer::singleShot(5000, [=](WMouseEvent) {
    finished().emit();
  });
}


MediaScannerDialog::MediaScannerDialog(Session* session, Settings* settings, MediaCollection* mediaCollection, WObject* parent)
    : d(new MediaScannerDialogPrivate(this))
{
  d->pages = {
    new ScanMediaInfoPage{session, mediaCollection},
    new CreateThumbnails{session, settings, mediaCollection},
    new FinalPage()
  };
  setWidth(700);
  setWindowTitle(wtr("mediascanner.title"));
  setClosable(false);
  footer()->addWidget(d->buttonNext = WW<WPushButton>(wtr("button.next")).css("btn btn-primary").setEnabled(false).onClick([=](WMouseEvent) {
    d->buttonNext->disable();
    d->widgetsStack->setCurrentIndex(d->widgetsStack->currentIndex() + 1);
    WTimer::singleShot(100, [=](WMouseEvent) { d->pages[d->widgetsStack->currentIndex()]->run(); });
  }));
  footer()->addWidget(d->buttonClose = WW<WPushButton>(wtr("button.close")).css("btn btn-success").onClick([=](WMouseEvent) { accept(); } ).setEnabled(false));
  contents()->addWidget(d->widgetsStack = new WStackedWidget());
  for(auto page: d->pages) {
    d->widgetsStack->addWidget(page);
    page->finished().connect([=](_n6) {
      if(d->widgetsStack->currentIndex() == d->pages.size()-1)
        d->buttonClose->enable();
      else
        d->buttonNext->enable();
    });
  }
}

MediaScannerDialog::~MediaScannerDialog()
{
  delete d;
}


void MediaScannerDialog::run()
{
  show();
  d->pages[0]->run();
}

