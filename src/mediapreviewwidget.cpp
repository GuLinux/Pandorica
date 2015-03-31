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
#include <fstream>


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
  shared_ptr<ImageBlob> imageBlob;
  WContainerWidget *fileUploadContainer;
  Http::Client httpClient;
  void imageDownloaded(const boost::system::error_code &errorCode, const Http::Message &message);
  void resetImageUploader();
private:
  MediaPreviewWidget *q;
};

MediaPreviewWidget::Private::Private(const Media& media, Session* session, MediaPreviewWidget* q)
  : app(wApp), media(media), session(session), image(WW<WImage>().css("image-responsive") ), fileUploadContainer(WW<WContainerWidget>().setInline(true))
{
  httpClient.setFollowRedirect(true);
  httpClient.setMaximumResponseSize(20 * 1024 * 1024);
  image->setMaximumSize(WLength::Auto, 450);
  image->setMargin(WLength::Auto, Side::Left);
  image->setMargin(WLength::Auto, Side::Right);
}

void MediaPreviewWidget::Private::imageDownloaded(const boost::system::error_code& errorCode, const Http::Message& message)
{

}

void MediaPreviewWidget::Private::resetImageUploader()
{
  fileUploadContainer->clear();
  WFileUpload *uploadCover = WW<WFileUpload>().css("inline-block input-sm");
  uploadCover->setProgressBar(new WProgressBar);
  uploadCover->setInline(true);
  uploadCover->changed().connect(uploadCover, &WFileUpload::upload);
  
  uploadCover->uploaded().connect([=](_n1){
    imageBlob = make_shared<ImageBlob>();
    ifstream spoolFile(uploadCover->spoolFileName(), ios::binary);
    std::copy(std::istreambuf_iterator<char>(spoolFile), std::istreambuf_iterator<char>(), back_inserter(*imageBlob));
    image->setImageLink(new WMemoryResource(Wt::Utils::guessImageMimeTypeData(*imageBlob), *imageBlob, q));
    resetImageUploader();
  });
  
  uploadCover->fileTooLarge().connect([=](int size, _n5){ wApp->log("notice") << "uploaded file too large (" << size << " bytes)"; resetImageUploader(); });
  fileUploadContainer->addWidget(uploadCover);
}



MediaPreviewWidget::~MediaPreviewWidget()
{

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
      auto image_data = ffmpegMedia.randomThumbnail();
      ImageBlob image_data_vector = *image_data;
      WServer::instance()->post(app->sessionId(), [=]{
        d->image->setImageLink(new WMemoryResource{Wt::Utils::guessImageMimeTypeData(image_data_vector), image_data_vector});
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

  
  Http::Client *client = new Http::Client(this);
  client->done().connect([=](const boost::system::error_code &error_code, const Http::Message &message, _n4){
    if(error_code.value() != boost::system::errc::success || message.status() < 200 || message.status() > 299) {
      wApp->log("notice") << "error loading image: " << error_code.message() << ", status=" << message.status();
      return;
    }
    string content_type = *message.getHeader("Content-Type");
    wApp->log("notice") << "content type: " << content_type;
    ImageBlob imageBlob;
    auto body = message.body();
    copy(body.begin(), body.end(), back_inserter(imageBlob));
    d->image->setImageLink(new WMemoryResource(content_type, imageBlob));
    wApp->triggerUpdate();
  });
  googlePicker->imageChosen().connect([=](const WString &url, _n5){
    wApp->log("notice") << "Got url: " << url;
      client->setFollowRedirect(true);
      client->setMaximumResponseSize(20 * 1024 * 1024);
      client->get(url.toUTF8());
  });
  content->addWidget(d->image);
  setImplementation(content);
}
