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

#include "createthumbnails.h"
#include "createthumbnails_p.h"

#include "ffmpegmedia.h"
#include "mediaattachment.h"
#include "session.h"
#include "sessioninfo.h"
#include "sessiondetails.h"
#include "comment.h"
#include <Wt/WProgressBar>
#include <Wt/WText>
#include <Wt/WPushButton>
#include <Wt/WTime>
#include <Wt/WMemoryResource>
#include <Wt/WImage>
#include "Wt-Commons/wt_helpers.h"
#include <mediacollection.h>
#include <player/html5player.h>
#include <settings.h>
#include <boost/thread.hpp>
#include <boost/format.hpp>

#include <libffmpegthumbnailer/videothumbnailer.h>
#include <libffmpegthumbnailer/videothumbnailerc.h>
#include <libffmpegthumbnailer/imagetypes.h>
#include <chrono>
#include <random>
#include <thread>

using namespace Wt;
using namespace std;
using namespace std::chrono;
using namespace ffmpegthumbnailer;


time_point<high_resolution_clock> serverStartTimeForRandomSeeding{high_resolution_clock::now()};
mt19937_64 randomEngine{(uint64_t) serverStartTimeForRandomSeeding.time_since_epoch().count()};

CreateThumbnailsPrivate::CreateThumbnailsPrivate(WPushButton* nextButton, WPushButton* retryButton, WApplication* app, Session* session, Settings* settings, CreateThumbnails* q)
  : nextButton(nextButton), retryButton(retryButton), app(app), session(session), settings(settings), q(q)
{
}
CreateThumbnailsPrivate::~CreateThumbnailsPrivate()
{
}

CreateThumbnails::CreateThumbnails(WPushButton* nextButton, WPushButton* retryButton, WApplication* app, Session* session, Settings* settings, WObject* parent)
  : WObject(parent), d(new CreateThumbnailsPrivate{nextButton, retryButton, app, session, settings, this})
{

}

CreateThumbnails::~CreateThumbnails()
{
    delete d;
}

#define defaultThumbnails(position) d->thumbnailFor(media, 640, position), d->thumbnailFor(media, 260, position, 3)
MediaScannerStep::StepResult CreateThumbnails::run(FFMPEGMedia* ffmpegMedia, Media* media, WContainerWidget* container)
{
  Dbo::Transaction t(*d->session);
  if(d->session->query<int>("SELECT COUNT(id) FROM media_attachment WHERE media_id = ? AND type = 'preview'").bind(media->uid()) > 0)
    return Skip;
  if(!ffmpegMedia->isVideo())
    return Skip;
  d->action = CreateThumbnailsPrivate::None;
  ThumbnailPosition position = d->randomPosition(ffmpegMedia);
  d->chooseRandomFrame(position, media, t, container);
  while(d->action == CreateThumbnailsPrivate::None) {
    this_thread::sleep_for(milliseconds(50));
  }
  guiRun(d->app, [=]{
    d->nextButton->disable();
    d->retryButton->disable();
    d->retryButtonConnection.disconnect();
    d->nextButtonConnection.disconnect();
    d->app->triggerUpdate();
  });
  if(d->action == CreateThumbnailsPrivate::NewRandom) {
    return ToRedo;
  }
  if(d->action == CreateThumbnailsPrivate::Accept) {
    d->saveThumbnails(media->uid(), defaultThumbnails(position), t);
    return Complete;
  }
  return Skip; // impossible?
}



void CreateThumbnailsPrivate::chooseRandomFrame(ThumbnailPosition position, Media* media, Dbo::Transaction& t, WContainerWidget* container)
{
  delete thumbnail;
  thumbnail = new WMemoryResource("image/png", thumbnailFor(media, 550, position), container);
  guiRun(app, [=]{
    container->clear();
    
    container->addWidget(WW<WContainerWidget>().add(new WImage(thumbnail)).setContentAlignment(AlignCenter));
    retryButtonConnection = retryButton->clicked().connect([=](WMouseEvent) {
      action = NewRandom;
    });
    nextButtonConnection = nextButton->clicked().connect([=](WMouseEvent) {
      action = Accept;
    });
    nextButton->enable();
    retryButton->enable();
    wApp->triggerUpdate();
  });
}



ThumbnailPosition CreateThumbnailsPrivate::randomPosition(FFMPEGMedia* ffmpegMedia)
{
  auto randomNumber = randomEngine();
  
  if(ffmpegMedia->durationInSeconds() < 100 ) {
    int percent = randomNumber % 100;
    if(percent < 10) percent += 10;
    if(percent > 80) percent -= 20;
    return {percent};
  }
  int percent{0};
  int position{0};
  while(percent < 10 || percent > 80) {
    randomNumber = randomEngine();
    position = randomNumber % ffmpegMedia->durationInSeconds();
    percent = position * 100.0 / ffmpegMedia->durationInSeconds();
  }
  return ThumbnailPosition::from(position);;
}

ThumbnailPosition ThumbnailPosition::from(int timeInSeconds)
{
  int remaining = 0;
  int hours = timeInSeconds / 3600;
  timeInSeconds %= 3600;
  int minutes = timeInSeconds / 60;
  timeInSeconds %= 60;
  string currentTimeStr = (boost::format("%.2d:%.2d:%.2d") %hours %minutes %timeInSeconds).str();
  return {-1, currentTimeStr};
}

void CreateThumbnailsPrivate::saveThumbnails(string mediaId, const std::vector<uint8_t> &forPlayer, const std::vector<uint8_t> &forThumbnail, Dbo::Transaction &t)
{
  session->add(new MediaAttachment{"preview", "thumbnail", "", mediaId, "image/png", forThumbnail });
  session->add(new MediaAttachment{"preview", "player", "", mediaId, "image/png", forPlayer });
  t.commit();
}

vector<uint8_t> CreateThumbnailsPrivate::thumbnailFor(Media* media, int size, ThumbnailPosition position, int quality)
{
  video_thumbnailer *thumbnailer = video_thumbnailer_create();
  thumbnailer->overlay_film_strip = media->mimetype().find("video") == string::npos ? 0 : 1;
  
  if(position.percent>0)
    thumbnailer->seek_percentage = position.percent;
  else
    thumbnailer->seek_time = (char*)position.timing.c_str();;
  
  thumbnailer->thumbnail_image_type = Png;
  
  image_data *imageData = video_thumbnailer_create_image_data();
  thumbnailer->thumbnail_image_quality = quality;
  thumbnailer->thumbnail_size = size;
  video_thumbnailer_generate_thumbnail_to_buffer(thumbnailer, media->fullPath().c_str(), imageData);
  vector<uint8_t> data{imageData->image_data_ptr, imageData->image_data_ptr + imageData->image_data_size};
  video_thumbnailer_destroy_image_data(imageData);
  video_thumbnailer_destroy(thumbnailer);
  return data;
}
