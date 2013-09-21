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




#include "MediaScanner/scanmediainfostep.h"
#include "private/scanmediainfostep_p.h"
#include <Wt/WProgressBar>
#include <Wt/WText>
#include <Wt/WApplication>
#include <Wt/WPushButton>
#include <Wt/WLineEdit>
#include <Wt/WTimer>
#include <Wt/WLabel>
#include <Wt/WTime>
#include "Wt-Commons/wt_helpers.h"
#include <session.h>
#include "media/mediacollection.h"
#include <thread>

#include "ffmpegmedia.h"
#include <Wt/Dbo/Session>

#include "utils/utils.h"


#include "Models/models.h"

using namespace Wt;
using namespace std;
using namespace std::chrono;
namespace fs = boost::filesystem;
using namespace PandoricaPrivate;
using namespace WtCommons;

ScanMediaInfoStepPrivate::ScanMediaInfoStepPrivate(ScanMediaInfoStep* q, WApplication *app)
  : q(q), app(app)
{
}
ScanMediaInfoStepPrivate::~ScanMediaInfoStepPrivate()
{
}


ScanMediaInfoStep::ScanMediaInfoStep(WApplication* app, WObject* parent)
  : WObject(parent), d(new ScanMediaInfoStepPrivate(this, app))
{
}


void ScanMediaInfoStep::run(FFMPEGMedia* ffmpegMedia, Media media, WContainerWidget* container, Dbo::Transaction* transaction, MediaScannerStep::ExistingFlags onExisting)
{
  d->result = Waiting;
  MediaPropertiesPtr mediaPropertiesPtr = media.properties(*transaction);
  if(onExisting == SkipIfExisting && mediaPropertiesPtr) {
    d->result = Skip;
    return;
  }
  string titleSuggestion = ffmpegMedia->metadata("title").empty() ? Utils::titleHintFromFilename(media.filename()) : ffmpegMedia->metadata("title");
  d->newTitle = titleSuggestion;
  d->ffmpegMedia = ffmpegMedia;
  d->media = media;
  guiRun(d->app, boost::bind(&ScanMediaInfoStepPrivate::setupGui, d, container, titleSuggestion));
  d->result = Done;
}
MediaScannerStep::StepResult ScanMediaInfoStep::result()
{
  return d->result;
}

void ScanMediaInfoStep::save(Dbo::Transaction* transaction)
{
  if(d->result != Done)
    return;
  transaction->session().execute("DELETE FROM media_properties WHERE media_id = ?").bind(d->media.uid());
  pair<int, int> resolution = d->ffmpegMedia->resolution();
  auto mediaProperties = new MediaProperties{d->media.uid(), d->newTitle, d->media.fullPath(), d->ffmpegMedia->durationInSeconds(), fs::file_size(d->media.path()), resolution.first, resolution.second};
  transaction->session().add(mediaProperties);
  d->result = Waiting;
}



void ScanMediaInfoStepPrivate::setupGui(Wt::WContainerWidget* container, std::string titleSuggestion)
{
  WLabel *label = new WLabel(wtr("mediascanner.media.title"));
  WLineEdit *editTitle = WW<WLineEdit>(titleSuggestion).css("span5");
  editTitle->changed().connect([=](_n1) {
    newTitle = editTitle->text().toUTF8();
  });
  label->setBuddy(editTitle);
  container->addWidget(WW<WContainerWidget>().css("form-inline").add(label).add(editTitle));
  app->triggerUpdate();
}



ScanMediaInfoStep::~ScanMediaInfoStep()
{
  delete d;
}

