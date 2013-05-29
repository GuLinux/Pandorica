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

#include "findorphansdialog.h"
#include "private/findorphansdialog_p.h"
#include "session.h"
#include <Wt/Dbo/collection>
#include <Wt/Dbo/SqlTraits>
#include <Wt/Dbo/StdSqlTraits>
#include <Wt/Dbo/Types>
#include <Wt/Dbo/Impl>


#include <Wt/Dbo/Query>
#include "mediacollection.h"
#include <Wt-Commons/wt_helpers.h>
#include "Models/mediaproperties.h"
#include "Models/mediaattachment.h"
#include "utils.h"
#include <Wt/WText>
#include <Wt/WTreeView>
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include <Wt/WPushButton>
#include <boost/thread.hpp>

#include "Models/models.h"

using namespace Wt;
using namespace std;
using namespace StreamingPrivate;
using namespace WtCommons;

FindOrphansDialogPrivate::FindOrphansDialogPrivate(FindOrphansDialog* q) : q(q)
{
}
FindOrphansDialogPrivate::~FindOrphansDialogPrivate()
{
}

FindOrphansDialog::~FindOrphansDialog()
{
    delete d;

}

FindOrphansDialog::FindOrphansDialog(MediaCollection *mediaCollection, Session *session, Wt::WObject* parent)
    : d(new FindOrphansDialogPrivate(this))
{
  d->mediaCollection = mediaCollection;
  d->session = session;
  contents()->addWidget(d->summary = WW<WText>().setInline(false));
  setClosable(false);
  
  WTreeView *view = new WTreeView();
  contents()->addWidget(view);
  d->model = new WStandardItemModel(0, 6, this);
  d->model->setHeaderData(0, Wt::Horizontal, wtr("findorphans.media.title"));
  d->model->setHeaderData(1, Wt::Horizontal, wtr("findorphans.attachment.type"));
  d->model->setHeaderData(2, Wt::Horizontal, wtr("findorphans.attachment.name"));
  d->model->setHeaderData(3, Wt::Horizontal, wtr("findorphans.attachment.value"));
  d->model->setHeaderData(4, Wt::Horizontal, wtr("findorphans.attachment.datasize"));
  view->setModel(d->model);
  view->resize(1000, 400);
  view->setColumnWidth(1, 75);
  view->setColumnWidth(2, 95);
  view->setColumnWidth(3, 65);
  view->setColumnWidth(4, 95);
  view->setColumnWidth(5, 60);
  footer()->addWidget(d->closeButton = WW<WPushButton>(wtr("button.close")).css("btn-danger").disable().onClick([=](WMouseEvent){ reject(); }));
  footer()->addWidget(d->saveButton = WW<WPushButton>(wtr("findorphans.removedata")).css("btn-primary").disable().onClick([=](WMouseEvent){ accept(); }));
  setWindowTitle(wtr("cleanup.orphans"));
  finished().connect([=](DialogCode code, _n5) {
    if(code == Accepted) {
      Dbo::Transaction t(*session);
      stringstream whereCondition;
      whereCondition <<"WHERE media_id in ( ";
      string separator;
      for(int row=0; row<d->model->rowCount(); row++) {
        WStandardItem *item = d->model->item(row);
        if(item->isChecked()) {
          whereCondition << separator << "'" << boost::any_cast<string>(item->data()) << "'";
          separator = ", ";
        }
      }
      whereCondition << ")";
      log("notice") << "executing statement: " << string{"DELETE FROM media_properties "} + whereCondition.str();
      d->session->execute(string{"DELETE FROM media_properties "} + whereCondition.str());
      d->session->execute(string{"DELETE FROM media_attachment "} + whereCondition.str());
      t.commit();
    }
  });
}

void FindOrphansDialog::run()
{
  show();
  boost::thread populateModelThread(boost::bind(&FindOrphansDialogPrivate::populateModel, d, d->model, wApp));
}

void FindOrphansDialogPrivate::populateModel(WStandardItemModel* model, WApplication* app)
{
  Session privateSession;
  Dbo::Transaction t(privateSession);
  mediaCollection->rescan(t);
  Dbo::collection<string> mediaIdsDbo = privateSession.query<string>("select media_id from media_attachment union select media_id from media_properties order by media_id").resultList();
  vector<string> mediaIds;
  remove_copy_if(mediaIdsDbo.begin(), mediaIdsDbo.end(), back_insert_iterator<vector<string>>(mediaIds), [=](string mediaId) { return mediaCollection->media(mediaId).valid(); } );
  
  for(string mediaId: mediaIds) {
    dataSummary.mediasCount++;
    MediaPropertiesPtr properties = privateSession.find<MediaProperties>().where("media_id = ?").bind(mediaId);
    WStandardItem *title = new WStandardItem{properties ? properties->title() : "<title unknown>"};
    title->setCheckable(true);
    title->setChecked(true);
    title->setData(mediaId);
    auto attachments = privateSession.find<MediaAttachment>().where("media_id = ?").bind(mediaId).resultList();
    for(MediaAttachmentPtr attachment: attachments) {
      dataSummary.attachmentsCount++;
      dataSummary.bytes += attachment->size();
      WStandardItem * type = new WStandardItem{attachment->type()};
      WStandardItem * name = new WStandardItem{attachment->name()};
      WStandardItem * value = new WStandardItem{attachment->value()};
      WStandardItem * size = new WStandardItem{ Utils::formatFileSize(attachment->size() ) };
      WStandardItem *link = new WStandardItem(wtr("findorphans.attachment.view"));
      link->setLink(attachment->link(attachment, model, false));
      title->appendRow({new WStandardItem, type, name, value, size, link});
    }
    WServer::instance()->post(app->sessionId(), [=] {
      model->appendRow(title);
      summary->setText(WString::trn("findorphans.summary", dataSummary.mediasCount).arg(dataSummary.mediasCount).arg(dataSummary.attachmentsCount).arg(Utils::formatFileSize(dataSummary.bytes)));
      app->triggerUpdate();
    });
    }
    WServer::instance()->post(app->sessionId(), [=] {
      closeButton->enable();
      saveButton->enable();
      summary->setText(WString::trn("findorphans.summary", dataSummary.mediasCount).arg(dataSummary.mediasCount).arg(dataSummary.attachmentsCount).arg(Utils::formatFileSize(dataSummary.bytes)));
      app->triggerUpdate();
    });
}

