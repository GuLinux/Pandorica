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
#include <Wt/WStackedWidget>
#include <Wt/WStringListModel>
#include <Wt/WComboBox>
#include <Wt/WProgressBar>
#include <Wt/WIOService>
#include <boost/format.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <thread>
#include "Models/models.h"
#include "settings.h"

using namespace Wt;
using namespace std;
using namespace PandoricaPrivate;
using namespace WtCommons;
namespace fs = boost::filesystem;

#define MAX_ULONG numeric_limits<uint64_t>::max()

FindOrphansDialogPrivate::FindOrphansDialogPrivate(FindOrphansDialog* q) : q(q)
{
  threadsSession = new Session();
}
FindOrphansDialogPrivate::~FindOrphansDialogPrivate()
{
  delete threadsSession;
}

FindOrphansDialog::~FindOrphansDialog()
{
    delete d;

}

FindOrphansDialog::FindOrphansDialog(MediaCollection* mediaCollection, Session* session, Settings* settings, WObject* parent)
    : d(new FindOrphansDialogPrivate(this))
{
  d->mediaCollection = mediaCollection;
  d->session = session;
  d->settings = settings;
  auto removeOrphansView = new WContainerWidget;
  d->stack = new WStackedWidget(contents());
  d->stack->setTransitionAnimation( {WAnimation::SlideInFromRight | WAnimation::Fade, WAnimation::EaseInOut, 500 });
  d->stack->addWidget(d->movedOrphansContainer = WW<WContainerWidget>()
    .add(WW<WContainerWidget>().add(new WText{wtr("cleanup.orphans.findmoved.title")}).setContentAlignment(AlignCenter))
    .add(WW<WText>(wtr("cleanup.orphans.findmoved.message")).css("small-text").setMargin(15, Side::Bottom))
  );
  
  d->stack->addWidget(WW<WContainerWidget>().setContentAlignment(AlignCenter)
    .add(new WText{wtr("cleanup.orphans.updatemoded.title")})
    .add(d->migrationProgress = new WProgressBar)
  );
  
  d->stack->addWidget(removeOrphansView);
  removeOrphansView->addWidget(WW<WContainerWidget>().add(new WText{wtr("cleanup.orphans.findorphans.title")}).setContentAlignment(AlignCenter) );
  removeOrphansView->addWidget(d->summary = WW<WText>().setInline(false).css("small-text"));
  setClosable(false);
  resize(1040, 600);
  
  WTreeView *view = new WTreeView();
  removeOrphansView->addWidget(view);
  d->model = new WStandardItemModel(0, 6, this);
  d->model->setHeaderData(0, Wt::Horizontal, wtr("findorphans.media.title"));
  d->model->setHeaderData(1, Wt::Horizontal, wtr("findorphans.attachment.type"));
  d->model->setHeaderData(2, Wt::Horizontal, wtr("findorphans.attachment.name"));
  d->model->setHeaderData(3, Wt::Horizontal, wtr("findorphans.attachment.value"));
  d->model->setHeaderData(4, Wt::Horizontal, wtr("findorphans.attachment.datasize"));
  view->setModel(d->model);
  view->resize(1000, 380);
  view->setColumnWidth(1, 75);
  view->setColumnWidth(2, 95);
  view->setColumnWidth(3, 65);
  view->setColumnWidth(4, 95);
  view->setColumnWidth(5, 60);
  footer()->addWidget(d->nextButton = WW<WPushButton>(wtr("button.next")).css("btn-warning").disable());
  d->nextButton->clicked().connect(this, &FindOrphansDialog::nextButtonClicked);
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
      d->session->execute(string{"DELETE FROM comment "} + whereCondition.str());
      d->session->execute(string{"DELETE FROM media_rating "} + whereCondition.str());
      t.commit();
    }
  });
}

void FindOrphansDialog::run()
{
  show();
  WServer::instance()->ioService().post(boost::bind(&FindOrphansDialogPrivate::populateMovedFiles, d, wApp));
}

void FindOrphansDialog::nextButtonClicked() {
  d->nextButton->disable();
  d->closeButton->disable();
  WServer::instance()->ioService().post(boost::bind(&FindOrphansDialogPrivate::applyMigrations, d, wApp));
}

void FindOrphansDialogPrivate::applyMigrations(WApplication* app)
{
  WServer::instance()->post(app->sessionId(), [=] {
    stack->setCurrentIndex(1);
    migrationProgress->setMaximum(migrations.size());
    app->triggerUpdate();
  });
  Dbo::Transaction t(*threadsSession);
  int currentMigration = 0;
  for(auto migration: migrations) {
    migration(t);
    currentMigration++;
    WServer::instance()->post(app->sessionId(), [=] {
      migrationProgress->setValue(currentMigration);
      app->triggerUpdate();
    });
  }
  t.commit();
  WServer::instance()->post(app->sessionId(), [=] {
    stack->setCurrentIndex(2);
    app->triggerUpdate();
  });
  populateRemoveOrphansModel(app);
}


std::vector< std::string > FindOrphansDialogPrivate::orphans(Dbo::Transaction& transaction)
{
  Dbo::collection<string> mediaIdsDbo = transaction.session().query<string>("select media_id from media_attachment union \
  select media_id from media_properties union \
  select media_id from comment union \
  select media_id from media_rating \
  order by media_id").resultList();
  vector<string> mediaIds;
  remove_copy_if(mediaIdsDbo.begin(), mediaIdsDbo.end(), back_insert_iterator<vector<string>>(mediaIds), [=](string mediaId) { return mediaCollection->media(mediaId).valid(); } );
  return mediaIds;
}


vector<string> tokenize(string filename) {
  vector<string> tokens;
  boost::regex re("(\\b|[-_.,;:])+");
  boost::sregex_token_iterator i(filename.begin(), filename.end(), re, -1);
  boost::sregex_token_iterator j;
  while(i!=j) {
    string token = *i++;
    if(!token.empty() && token.size() > 2)
      tokens.push_back(token);
  }
  return tokens;
}

FileSuggestion::FileSuggestion(string filePath, string mediaId, vector<string> originalFileTokens)
  : filePath(filePath), mediaId(mediaId)
{
  string filename = fs::path(filePath).filename().string();
  vector<string> myTokens = tokenize(filename);
  for(string token: myTokens) {
    int occurrences = count(originalFileTokens.begin(), originalFileTokens.end(), token);
    score += occurrences;
  }
}


void FindOrphansDialogPrivate::populateMovedFiles(WApplication* app)
{
  Dbo::Transaction t(*threadsSession);
  mediaCollection->rescan(t);
  int migrationsFound = 0;
  Dbo::collection<MediaPropertiesPtr> allMedias = threadsSession->find<MediaProperties>().resultList();
  for(MediaPropertiesPtr media: allMedias) {
    auto collectionMedia = mediaCollection->media(media->mediaId());
    if(collectionMedia.valid() && media->filename() != collectionMedia.fullPath() ) {
      media.modify()->setFileName(collectionMedia.fullPath());
    }
    if(collectionMedia.valid() || media->filename().empty() ) continue;
    string originalFilePath = media->filename();
    string originalMediaId = media->mediaId();
    fs::path dbFileName = fs::path(originalFilePath).filename();
    vector<string> originalFileTokens = tokenize(dbFileName.string());
    vector<FileSuggestion> suggestions;
    
    for(auto collectionMedia: mediaCollection->collection()) {
      string filePath = collectionMedia.second.path().string();
      if(collectionMedia.second.path().filename() == dbFileName) {
        suggestions.push_back({filePath, collectionMedia.first, MAX_ULONG});
      } else {
        FileSuggestion suggestion{filePath, collectionMedia.first, originalFileTokens};
        if(suggestion.score>0)
          suggestions.push_back(suggestion);
      }
    }
    if(suggestions.size() == 0)
      continue;
    sort(suggestions.begin(), suggestions.end(), [](FileSuggestion a, FileSuggestion b) { return a.score > b.score; });
    WStringListModel *model = new WStringListModel(q);
    
    model->addString("Do Nothing");
    model->setData(model->rowCount()-1, 0, string{} , MediaId);
    int selection = 0;
    
    for(FileSuggestion suggestion: suggestions) {
      model->addString(settings->relativePath(suggestion.filePath, threadsSession, true) + (suggestion.score == MAX_ULONG ? " (best match)" : "") );
      model->setData(model->rowCount()-1, 0, suggestion.mediaId, MediaId);
      model->setData(model->rowCount()-1, 0, suggestion.score, Score);
      model->setData(model->rowCount()-1, 0, suggestion.filePath, Path);
      if(selection == 0 && suggestion.score == MAX_ULONG)
        selection = model->rowCount()-1;
      if(model->rowCount() > 4) break;
    }
    migrationsFound++;
    WServer::instance()->post(app->sessionId(), [=] {
      WContainerWidget *fileContainer = new WContainerWidget(movedOrphansContainer);
      fileContainer->addWidget(WW<WText>(settings->relativePath(originalFilePath, session, true)).css("small-text"));
      WComboBox *combo = WW<WComboBox>(fileContainer).css("input-block-level small-text");
      migrations.push_back([=](Dbo::Transaction &t) {
        boost::any mediaIdModelData = model->data(model->index(combo->currentIndex(), 0), MediaId);
        log("notice") << "type for mediaIdModelData: " << mediaIdModelData.type().name();
        string newMediaId = boost::any_cast<string>(mediaIdModelData);
        if(newMediaId.empty()) {
          log("notice") << "No new media id for " << originalFilePath;
          return;
        }
        log("notice") << "Migrating " << originalFilePath << " to " << newMediaId;
        migrate(t, originalMediaId, newMediaId);
      });
      combo->setModel(model);
      combo->setCurrentIndex(selection);
      app->triggerUpdate();
    });
  }
  while(migrations.size() != migrationsFound)
    this_thread::sleep_for(chrono::milliseconds(100));
  bool skipToRemoveOrphans = migrations.empty();
  WServer::instance()->post(app->sessionId(), [=] {
    nextButton->setEnabled(!skipToRemoveOrphans);
    closeButton->setEnabled(!skipToRemoveOrphans);
    if(skipToRemoveOrphans) {
      stack->setCurrentIndex(2);
    }
    app->triggerUpdate();
  });
  if(skipToRemoveOrphans)
    populateRemoveOrphansModel(app);
}


void FindOrphansDialogPrivate::migrate(Dbo::Transaction& transaction, string oldMediaId, string newMediaId)
{
  string migrationQuery{"UPDATE %s SET media_id = ? WHERE media_id = ?"};
  MediaPropertiesPtr mediaProperties = transaction.session().find<MediaProperties>().where("media_id = ?").bind(newMediaId);
  if(mediaProperties) {
    log("notice") << "Media properties already found, skipping and deleting original";
    transaction.session().execute("DELETE FROM media_properties WHERE media_id = ?").bind(oldMediaId);
  }
  else {
    transaction.session().execute( (boost::format(migrationQuery) % "media_properties").str() ).bind(newMediaId).bind(oldMediaId);
  }
  Dbo::collection<MediaAttachmentPtr> attachments = transaction.session().find<MediaAttachment>().where("media_id = ?").bind(newMediaId);
  if(attachments.size() > 0) {
    log("notice") << "Media attachments already found, skipping and deleting original";
    transaction.session().execute("DELETE FROM media_attachment WHERE media_id = ?").bind(oldMediaId);
  } else {
    transaction.session().execute( (boost::format(migrationQuery) % "media_attachment").str() ).bind(newMediaId).bind(oldMediaId);
  }
  transaction.session().execute( (boost::format(migrationQuery) % "comment").str() ).bind(newMediaId).bind(oldMediaId);
  transaction.session().execute( (boost::format(migrationQuery) % "media_rating").str() ).bind(newMediaId).bind(oldMediaId);
}


void FindOrphansDialogPrivate::populateRemoveOrphansModel(Wt::WApplication* app)
{
  Dbo::Transaction t(*threadsSession);
  mediaCollection->rescan(t);
  vector<string> mediaIds = orphans(t);
  
  for(string mediaId: mediaIds) {
    dataSummary.mediasCount++;
    MediaPropertiesPtr properties = threadsSession->find<MediaProperties>().where("media_id = ?").bind(mediaId);
    WStandardItem *title = new WStandardItem{properties ? properties->title() : "<title unknown>"};
    title->setCheckable(true);
    title->setChecked(true);
    title->setData(mediaId);
    auto attachments = threadsSession->find<MediaAttachment>().where("media_id = ?").bind(mediaId).resultList();
    for(MediaAttachmentPtr attachment: attachments) {
      dataSummary.attachmentsCount++;
      dataSummary.bytes += attachment->size();
      WStandardItem * type = new WStandardItem{attachment->type()};
      WStandardItem * name = new WStandardItem{attachment->name()};
      WStandardItem * value = new WStandardItem{attachment->value()};
      WStandardItem * size = new WStandardItem{ Utils::formatFileSize(attachment->size() ) };
      WStandardItem *link = new WStandardItem(wtr("findorphans.attachment.view"));
      link->setLink(attachment->link(attachment, t, model, false));
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
      saveButton->setEnabled(dataSummary.mediasCount > 0);
      summary->setText(WString::trn("findorphans.summary", dataSummary.mediasCount).arg(dataSummary.mediasCount).arg(dataSummary.attachmentsCount).arg(Utils::formatFileSize(dataSummary.bytes)));
      app->triggerUpdate();
    });
}

