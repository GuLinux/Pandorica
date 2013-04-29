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
    
    video_thumbnailer *thumbnailer = video_thumbnailer_create();
    thumbnailer->overlay_film_strip = 1;
    
    if(((int) currentTime) > 0) {
      int hours = currentTime / 3600;
      int minutes = (int(currentTime)% 3600) / 60;
      int seconds = ((int(currentTime) % 3600) % 60) /60;
      string currentTimeStr = (boost::format("%.2d:%.2d:%.2d") %hours %minutes %seconds).str();
      thumbnailer->seek_time = (char*)currentTimeStr.c_str();
    } else
      thumbnailer->seek_percentage = 20;
    thumbnailer->thumbnail_image_type = Png;
    
    auto savePreview = [=] (string size, image_data *imageData) {
      vector<unsigned char> data{imageData->image_data_ptr, imageData->image_data_ptr+ imageData->image_data_size};
      session->add(new MediaAttachment{"preview", size, "", media.first, "image/png", data });
    };
    
    image_data *imageData = video_thumbnailer_create_image_data();
    thumbnailer->thumbnail_image_quality = 8;
    thumbnailer->thumbnail_size = 640;
    video_thumbnailer_generate_thumbnail_to_buffer(thumbnailer, media.second.fullPath().c_str(), imageData);
    savePreview("player", imageData);

    
    
    thumbnailer->thumbnail_image_quality = 4;
    thumbnailer->thumbnail_size = 260;
    video_thumbnailer_generate_thumbnail_to_buffer(thumbnailer, media.second.fullPath().c_str(), imageData);
    savePreview("thumbnail", imageData);
    
    video_thumbnailer_destroy(thumbnailer);
    
    /*
    VideoThumbnailer thumbnailer;
    thumbnailer.setThumbnailSize(640);
    thumbnailer.setImageQuality(5);
    thumbnailer.setWorkAroundIssues(true);
    
    thumbnailer.setSeekPercentage(currentTime / duration * 100);
    thumbnailer.setSmartFrameSelection(true);
    thumbnailer.generateThumbnail(media.second.fullPath(), Png, "/tmp/autopreview.png");
    */
    
  }
  guiRun([=] {
    contentForEachMedia->clear();
    updateGuiProgress(current, "");
    q->finished().emit();
    wApp->triggerUpdate();
  });
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
