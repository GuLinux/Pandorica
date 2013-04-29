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
#include <session.h>
#include <mediacollection.h>
#include "Wt-Commons/wt_helpers.h"
#include <Wt/WPushButton>
#include <Wt/WStackedWidget>

using namespace Wt;
using namespace std;

MediaScannerDialogPrivate::MediaScannerDialogPrivate(MediaScannerDialog* q) : q(q)
{
}
MediaScannerDialogPrivate::~MediaScannerDialogPrivate()
{
}

MediaScannerDialog::MediaScannerDialog(Session* session, MediaCollection* mediaCollection, Wt::WObject* parent)
    : d(new MediaScannerDialogPrivate(this))
{
  d->pages = {
    new ScanMediaInfoPage{session, mediaCollection},
  };
  setWidth(650);
  setWindowTitle(wtr("mediascanner.title"));
  setClosable(false);
  footer()->addWidget(d->buttonNext = WW<WPushButton>(wtr("button.next")).css("btn btn-primary").setEnabled(false).onClick([=](WMouseEvent) {
    d->buttonNext->disable();
    d->widgetsStack->setCurrentIndex(d->widgetsStack->currentIndex() + 1);
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

