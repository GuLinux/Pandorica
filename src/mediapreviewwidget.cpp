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
#include "utils/image.h"
#include "session.h"

using namespace std;
using namespace Wt;
using namespace WtCommons;

MediaPreviewWidget::~MediaPreviewWidget()
{

}

MediaPreviewWidget::MediaPreviewWidget(const Media& media, Session *session, WContainerWidget* parent) : WCompositeWidget(parent)
{
  Dbo::Transaction t(*session);
  WContainerWidget *content = WW<WContainerWidget>();
  content->setLayout(new WVBoxLayout);
  WFileUpload *uploadCover = WW<WFileUpload>().css("inline-block");
  uploadCover->setInline(true);
  GooglePicker *googlePicker = new GooglePicker{"Load from Google Images"};
  googlePicker->searchString(media.title(t));
  content->layout()->addWidget(WW<WContainerWidget>().add(uploadCover).add(googlePicker));
  WImage *image = WW<WImage>().css("img-responsive");
  auto image_from_db = media.preview(t, Media::PreviewFull);
  if(image_from_db) {
    image->setImageLink(image_from_db->link(image_from_db, t, image));
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
    image->setImageLink(new WMemoryResource(content_type, imageBlob));
    wApp->triggerUpdate();
  });
  googlePicker->imageChosen().connect([=](const WString &url, _n5){
    wApp->log("notice") << "Got url: " << url;
      client->setFollowRedirect(true);
      client->setMaximumResponseSize(20 * 1024 * 1024);
      client->get(url.toUTF8());
  });
  content->layout()->addWidget(image);
  setImplementation(content);
}
