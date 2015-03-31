/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "src/mediapreviewwidget.h"
#include <Wt/WContainerWidget>
#include <Wt/WVBoxLayout>
#include "googlepicker.h"
#include "media/media.h"
#include "Wt-Commons/wt_helpers.h"
#include <Wt-Commons/wobjectscope.h>
#include <Wt/WFileUpload>
#include <Wt/Dbo/Transaction>
#include <Wt/WImage>
#include <Wt/WMemoryResource>
#include <Wt/Http/Client>
#include <Wt/WFileResource>
#include <Wt/WPushButton>
#include <Wt/Utils>
#include <Wt/WToolBar>
#include <Wt/WProgressBar>
#include "utils/image.h"
#include "session.h"
#include "threadpool.h"
#include "ffmpegmedia.h"
#include "utils/d_ptr_implementation.h"
#include "utils/utils.h"
#include "pandorica.h"
#include <fstream>
#include <boost/format.hpp>


using namespace std;
using namespace Wt;
using namespace WtCommons;

class MediaPreviewWidget::Private {
public:
  Private(const Media &media, Session *session, MediaPreviewWidget *q);
  WApplication *app;
  Media media;
  Session *session;
  
  WImage *image;
  WContainerWidget *fileUploadContainer;
  Http::Client httpClient;
  void resetImageUploader();
  void updateImage(const ImageBlob& imageblob);
  shared_ptr<WMemoryResource> imageResource;
private:
  MediaPreviewWidget *q;
};

MediaPreviewWidget::Private::Private(const Media& media, Session* session, MediaPreviewWidget* q)
  : app(wApp), media(media), session(session), image(WW<WImage>().css("img-responsive") ), fileUploadContainer(WW<WContainerWidget>().setInline(true))
{
  httpClient.setFollowRedirect(true);
  httpClient.setMaximumResponseSize(20 * 1024 * 1024);
  httpClient.done().connect([=](const boost::system::error_code &error_code, const Http::Message &message, _n4){
    if(error_code.value() != boost::system::errc::success || message.status() < 200 || message.status() > 299) {
      wApp->log("notice") << "error loading image: " << error_code.message() << ", status=" << message.status();
      Pandorica::instance()->notify(WString::tr("media.image.download.error"), Pandorica::Alert, 10);
      app->triggerUpdate();
      return;
    }
    ImageBlob imageBlob;
    ::Utils::copy(message.body(), imageBlob);
    updateImage(imageBlob);
    app->triggerUpdate();
  });
  image->setMaximumSize(WLength::Auto, 450);
  image->setMargin(WLength::Auto, Side::Left);
  image->setMargin(WLength::Auto, Side::Right);
}


void MediaPreviewWidget::Private::updateImage(const ImageBlob &imageblob)
{
  image->setImageLink(( imageResource = make_shared<WMemoryResource>(Wt::Utils::guessImageMimeTypeData(imageblob), imageblob, image) ).get() );
}


void MediaPreviewWidget::Private::resetImageUploader()
{
  fileUploadContainer->clear();
  WFileUpload *uploadCover = WW<WFileUpload>().css("inline-block input-sm");
  uploadCover->setProgressBar(new WProgressBar);
  uploadCover->setInline(true);
  uploadCover->changed().connect(uploadCover, &WFileUpload::upload);
  
  uploadCover->uploaded().connect([=](_n1){
    ImageBlob imageBlob;
    ifstream spoolFile(uploadCover->spoolFileName(), ios::binary);
    std::copy(std::istreambuf_iterator<char>(spoolFile), std::istreambuf_iterator<char>(), back_inserter(imageBlob));
    updateImage(imageBlob);
    resetImageUploader();
  });
  
  uploadCover->fileTooLarge().connect([=](int size, _n5){ wApp->log("notice") << "uploaded file too large (" << size << " bytes)"; resetImageUploader(); });
  fileUploadContainer->addWidget(uploadCover);
}



MediaPreviewWidget::~MediaPreviewWidget()
{
}

void MediaPreviewWidget::save()
{
  if(! d->imageResource ) return;
  Dbo::Transaction t(*d->session);
  d->session->execute((boost::format("DELETE FROM %s WHERE media_id = ?") % d->session->tableName<MediaAttachment>()).str()).bind(d->media.uid());
  d->media.setImage(make_shared<Image>(d->imageResource->data()), t);
}


MediaPreviewWidget::MediaPreviewWidget(const Media& media, Session *session, WContainerWidget* parent) : WCompositeWidget(parent), d(media, session, this)
{
  WApplication *app = wApp;
  Dbo::Transaction t(*session);
  WContainerWidget *content = WW<WContainerWidget>();

  
  GooglePicker *googlePicker = new GooglePicker{"Load from Google Images"};
  googlePicker->searchString(media.title(t));
  WPushButton *loadFromFile = WW<WPushButton>(WString::tr("media.image.fromfile")).css("btn-xs").onClick([=](WMouseEvent){
    ThreadPool::instance()->post([=]{
      FFMPEGMedia ffmpegMedia(media);
      auto image = ffmpegMedia.randomThumbnail();
      WServer::instance()->post(app->sessionId(), [=]{
        d->updateImage(*image);
        app->triggerUpdate();
      });
    });
  });
  
  WToolBar *toolbar = WW<WToolBar>().css("inline-block");
  WPushButton *googlePickerButton = *googlePicker;
  googlePickerButton->addStyleClass("btn-xs");
  if(googlePickerButton)
    toolbar->addButton(googlePickerButton);
  toolbar->addButton(loadFromFile);
  
  content->addWidget(WW<WContainerWidget>().add(d->fileUploadContainer).add(toolbar));
  d->resetImageUploader();

  auto image_from_db = media.preview(t, Media::PreviewFull);
  
  if(image_from_db) {
    d->image->setImageLink(image_from_db->link(image_from_db, t, d->image));
  }

  
  googlePicker->imageChosen().connect([=](const WString &url, _n5){
    wApp->log("notice") << "Got url: " << url;
    d->httpClient.get(url.toUTF8());
  });
  content->addWidget(d->image);
  setImplementation(content);
}
