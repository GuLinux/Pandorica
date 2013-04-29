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

using namespace Wt;
using namespace std;
using namespace ffmpegthumbnailer;

CreateThumbnailsPrivate::CreateThumbnailsPrivate(Session *session, MediaCollection *mediaCollection, Settings *settings, CreateThumbnails* q)
  : session(session), mediaCollection(mediaCollection), settings(settings), q(q)
{
}
CreateThumbnailsPrivate::~CreateThumbnailsPrivate()
{
}

CreateThumbnails::~CreateThumbnails()
{
    delete d;
}

void CreateThumbnails::run()
{
  d->progressBar->setMaximum(d->mediaCollection->collection().size());
  boost::thread t(boost::bind(&CreateThumbnailsPrivate::scanMedias, d, wApp, [=](int progress, string file) {
    d->progressBar->setValue(progress);
    d->progressBarTitle->setText(file);
    d->contentForEachMedia->clear();
    wApp->triggerUpdate();
  }));

}

#define guiRun(f) WServer::instance()->post(app->sessionId(), f)


void CreateThumbnailsPrivate::scanMedias(Wt::WApplication* app, UpdateGuiProgress updateGuiProgress)
{
  int current{0};
  for(auto media: mediaCollection->collection()) {
    guiRun(boost::bind(updateGuiProgress, ++current, media.second.filename()));
    
    Dbo::Transaction t(*session);
    if(session->query<int>("SELECT COUNT(*) FROM media_attachment WHERE media_id = ?").bind(media.first) > 0)
      continue;
    FFMPEGMedia ffmpegMedia{media.second};
    if(!ffmpegMedia.isVideo())
      continue;
    currentTime = -1;
    
    guiRun([=]{
      updateGuiProgress(current, media.second.filename());
    });
    chooseRandomFrame(media.second, t, app);
    continue;
    guiRun([=]{
      updateGuiProgress(current, media.second.filename());
      HTML5Player *player = new HTML5Player();
      contentForEachMedia->addWidget(player);
      player->addSource({settings->linkFor(media.second.path()).url(), media.second.mimetype() });
      contentForEachMedia->addWidget(WW<WPushButton>("Create Thumbnail").onClick([=](WMouseEvent){
        player->pause();
        player->getCurrentTime([=](double currentMediaTime, double mediaDuration, _n4) {
          this->currentTime = currentMediaTime;
          this->duration = mediaDuration;
        });
      }));
      wApp->triggerUpdate();
    });
    
    
    while(currentTime < 0) {
      boost::this_thread::sleep(boost::posix_time::millisec(100));
    }
    
    auto savePreview = [=] (string size, const vector<uint8_t> &data) {
      session->add(new MediaAttachment{"preview", size, "", media.first, "image/png", data });
    };
    savePreview("player", thumbnailFor(media.second, 640, ThumbnailPosition::from(currentTime)));
    savePreview("thumbnail", thumbnailFor(media.second, 260, ThumbnailPosition::from(currentTime), 3));
    t.commit();
  }
  guiRun([=] {
    contentForEachMedia->clear();
    updateGuiProgress(current, "");
    q->finished().emit();
    wApp->triggerUpdate();
  });
}

#define defaultThumbnails(position) thumbnailFor(media, 640, position), thumbnailFor(media, 260, position, 3)

void CreateThumbnailsPrivate::chooseFromVideoPlayer(const Media& media, Dbo::Transaction& t, WApplication *app)
{
  
}

void CreateThumbnailsPrivate::chooseRandomFrame(const Media& media, Dbo::Transaction& t, WApplication *app)
{
  long range = randomEngine.max() - randomEngine.min();
  int randomPercent = (double( randomEngine() - randomEngine.min()) / range * 100);
  if(randomPercent < 10) randomPercent += 10;
  if(randomPercent > 80) randomPercent -= 20;
  delete thumbnail;
  thumbnail = new WMemoryResource("image/png", thumbnailFor(media, 400, {randomPercent}), q);
  guiRun([=]{
    contentForEachMedia->clear();
    
    contentForEachMedia->addWidget(WW<WContainerWidget>().add(new WImage(thumbnail)).setContentAlignment(AlignCenter));
    contentForEachMedia->addWidget(WW<WPushButton>("New Random Image").css("btn btn-primary").onClick([=](WMouseEvent) {
      action = NewRandom;
    }));
    contentForEachMedia->addWidget(WW<WPushButton>("Accept").css("btn btn-success").onClick([=](WMouseEvent) {
      action = Accept;
    }));
    wApp->triggerUpdate();
  });
  action = None;
  while(action == None) {
    boost::this_thread::sleep(boost::posix_time::millisec(100));
  }
  if(action == NewRandom) {
    chooseRandomFrame(media, t, app);
    return;
  }
  if(action == Accept) {
    saveThumbnails(media.uid(), defaultThumbnails({randomPercent}), t);
  }
}

void CreateThumbnailsPrivate::saveThumbnails(string mediaId, const std::vector<uint8_t> &forPlayer, const std::vector<uint8_t> &forThumbnail, Dbo::Transaction &t)
{
  session->add(new MediaAttachment{"preview", "thumbnail", "", mediaId, "image/png", forThumbnail });
  session->add(new MediaAttachment{"preview", "player", "", mediaId, "image/png", forPlayer });
  t.commit();
}


ThumbnailPosition ThumbnailPosition::from(int timeInSeconds)
{
  int hours = timeInSeconds / 3600;
  int minutes = (int(timeInSeconds)% 3600) / 60;
  int seconds = ((int(timeInSeconds) % 3600) % 60) /60;
  string currentTimeStr = (boost::format("%.2d:%.2d:%.2d") %hours %minutes %seconds).str();
  return {-1, currentTimeStr};
}


vector<uint8_t> CreateThumbnailsPrivate::thumbnailFor(const Media& media, int size, ThumbnailPosition position, int quality)
{
  video_thumbnailer *thumbnailer = video_thumbnailer_create();
  thumbnailer->overlay_film_strip = media.mimetype().find("video") == string::npos ? 0 : 1;
  
  if(position.percent>0)
    thumbnailer->seek_percentage = position.percent;
  else
    thumbnailer->seek_time = (char*)position.timing.c_str();;
  
  thumbnailer->thumbnail_image_type = Png;
  
  image_data *imageData = video_thumbnailer_create_image_data();
  thumbnailer->thumbnail_image_quality = quality;
  thumbnailer->thumbnail_size = size;
  video_thumbnailer_generate_thumbnail_to_buffer(thumbnailer, media.fullPath().c_str(), imageData);
  vector<uint8_t> data{imageData->image_data_ptr, imageData->image_data_ptr + imageData->image_data_size};
  video_thumbnailer_destroy_image_data(imageData);
  video_thumbnailer_destroy(thumbnailer);
  return data;
}




CreateThumbnails::CreateThumbnails(Session* session, Settings *settings, MediaCollection* mediaCollection, WContainerWidget* parent)
    : MediaScannerPage(parent), d(new CreateThumbnailsPrivate(session, mediaCollection, settings, this))
{
  d->progressBar = new WProgressBar();
  d->progressBarTitle = new WText;
  addWidget(WW<WContainerWidget>().add(d->progressBarTitle).add(new WBreak).add(d->progressBar).setContentAlignment(AlignmentFlag::AlignCenter));
  d->contentForEachMedia = new WContainerWidget();
  addWidget(d->contentForEachMedia);
}
