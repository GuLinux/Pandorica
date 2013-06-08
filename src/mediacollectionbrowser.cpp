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


#include "mediacollectionbrowser.h"
#include <Wt/WMenu>
#include <Wt/WAnchor>
#include <Wt/WText>
#include <Wt/WPopupMenu>
#include <Wt/WImage>
#include <Wt/WMemoryResource>
#include "Wt-Commons/wt_helpers.h"
#include "Wt-Commons/wt_helpers.h"
#include "settings.h"
#include "session.h"
#include "utils.h"
#include <boost/format.hpp>
#include <algorithm>
#include <Wt/Dbo/Transaction>
#include <Wt/WTime>
#include <Wt/WMessageBox>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WFileResource>
#include <Wt/WVBoxLayout>
#include <Wt/WHBoxLayout>
#include "private/mediacollectionbrowser_p.h"
#include "Models/models.h"
#include "MediaScanner/createthumbnails.h"
#include "ffmpegmedia.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include <Wt/WPanel>
#include <Wt/WGroupBox>
#include <Wt/WTimer>

#define ROOT_PATH_ID "///ROOT///"

using namespace Wt;
using namespace std;
using namespace boost;
using namespace StreamingPrivate;
namespace fs = boost::filesystem;
using namespace WtCommons;

MediaCollectionBrowser::MediaCollectionBrowser(MediaCollection* collection, Settings* settings, Session* session, WContainerWidget* parent)
  : WContainerWidget(parent), d(new MediaCollectionBrowserPrivate(collection, settings, session, this))
{
  d->breadcrumb = WW<WContainerWidget>().css("breadcrumb");
  d->breadcrumb->setList(true);
  d->browser = WW<WContainerWidget>().css("thumbnails").setMargin(WLength::Auto, Left).setMargin(WLength::Auto, Right);
  WContainerWidget *mainContainer = new WContainerWidget;
  d->infoPanel = new InfoPanel(session, settings);

/*  
  mainContainer->setStyleClass("row-fluid");
  mainContainer->addWidget(d->infoPanel);
  d->infoPanel->addStyleClass("span2");
  mainContainer->addWidget(d->browser);
  d->browser->addStyleClass("span10");
*/
  auto layout = new WHBoxLayout();
  mainContainer->setLayout(layout);
  layout->addWidget(d->infoPanel);
  layout->setResizable(0, true);
  d->infoPanel->setWidth(450);
  layout->addWidget(WW<WContainerWidget>().css("mediabrowser").add(d->browser), 1);

  d->browser->setList(true);
  addWidget(d->breadcrumb);
  addWidget(mainContainer);
  d->currentPath = new RootCollectionPath{settings, session, collection};
  d->collectionPaths[ROOT_PATH_ID] = d->currentPath;
  collection->scanned().connect(this, &MediaCollectionBrowser::reload);
  d->infoPanel->play().connect([=](Media media, _n5) { d->playSignal.emit(media); });
  d->infoPanel->queue().connect([=](Media media, _n5) { d->queueSignal.emit(media); });
  d->infoPanel->setTitle().connect(d, &MediaCollectionBrowserPrivate::setTitleFor);
  d->infoPanel->setPoster().connect(d, &MediaCollectionBrowserPrivate::setPosterFor);
  d->infoPanel->deletePoster().connect(d, &MediaCollectionBrowserPrivate::clearThumbnailsFor);
}

void MediaCollectionBrowser::reload()
{
  d->browse(d->currentPath);
}



InfoPanel::InfoPanel(Session *session, Settings *settings, Wt::WContainerWidget* parent): WContainerWidget(parent), session(session), settings(settings)
{
  setStyleClass("browser-info-panel");
  isAdmin = session->user()->isAdmin();
  reset();
}


void InfoPanel::reset()
{
  clear();
  addWidget(WW<WContainerWidget>().setContentAlignment(AlignCenter)
    .add(WW<WText>(wtr("infopanel.empty.title")).css("media-title"))
    .add(WW<WImage>(Settings::staticPath("/icons/site-logo-big.png")))
  );
  addWidget(new WBreak);
  addWidget(WW<WText>(wtr("infopanel.empty.message")));
}


void InfoPanel::info(Media media)
{
  clear();
  WContainerWidget *header = WW<WContainerWidget>().setContentAlignment(AlignCenter);
  Dbo::Transaction t(*session);
  WString title = media.title(t);
  header->addWidget(WW<WText>(title).css("media-title"));
  Dbo::ptr<MediaAttachment> previewAttachment = media.preview(t, Media::PreviewPlayer);
  if(previewAttachment) {
    WLink previewLink = previewAttachment->link(previewAttachment, t, header);
    WLink fullImage = previewLink;
    
    Dbo::ptr<MediaAttachment> fullImageAttachment = media.preview(t, Media::PreviewFull);
    if(fullImageAttachment)
      fullImage = fullImageAttachment->link(fullImageAttachment, t, header);
    
    WAnchor *fullImageLink = new WAnchor{fullImage, new WImage{previewLink, title}};
    fullImageLink->setTarget(Wt::AnchorTarget::TargetNewWindow);
    header->addWidget(fullImageLink);
  } else {
    auto iconType = (media.mimetype().find("video") == string::npos) ? Settings::AudioFile : Settings::VideoFile;
    WImage *icon = new WImage{ settings->icon(iconType) };
    header->addWidget(icon);
  }
  auto mediaInfoPanel = createPanel("mediabrowser.information");
  
  mediaInfoPanel.second->addWidget(labelValueBox("mediabrowser.filename", media.filename()) );
  mediaInfoPanel.second->addWidget(labelValueBox("mediabrowser.filesize", Utils::formatFileSize(fs::file_size(media.path())) ) );
  
  MediaPropertiesPtr mediaProperties = media.properties(t);
  if(mediaProperties) {
    mediaInfoPanel.second->addWidget(labelValueBox("mediabrowser.medialength", WTime(0,0,0).addSecs(mediaProperties->duration()).toString()) );
    if(media.mimetype().find("video") != string::npos && mediaProperties->width() > 0 && mediaProperties->height() > 0)
      mediaInfoPanel.second->addWidget(labelValueBox("mediabrowser.resolution", WString("{1}x{2}").arg(mediaProperties->width()).arg(mediaProperties->height()) ) );
  }
  Ratings rating = MediaRating::ratingFor(media, t);
  if(rating.users > 0) {
    WContainerWidget *avgRatingWidget = new WContainerWidget;
    for(int i=1; i<=5; i++) {
      avgRatingWidget->addWidget(WW<WImage>(Settings::staticPath("/icons/rating_small.png")).css(rating.ratingAverage <i ? "rating-unrated" : ""));
    }
    mediaInfoPanel.second->addWidget(labelValueBox("mediabrowser.rating", avgRatingWidget));
  }
  
  auto actions = createPanel("mediabrowser.actions");
  actions.second->addWidget(WW<WPushButton>(wtr("mediabrowser.play")).css("btn btn-block btn-small btn-primary").onClick([=](WMouseEvent){ _play.emit(media);} ));
  actions.second->addWidget(WW<WPushButton>(wtr("mediabrowser.queue")).css("btn btn-block btn-small").onClick([=](WMouseEvent){ _queue.emit(media);} ));
  actions.second->addWidget(WW<WPushButton>(wtr("mediabrowser.share")).css("btn btn-block btn-small").onClick([=](WMouseEvent){
    Wt::Dbo::Transaction t(*session);
    auto shareMessageBox = new WMessageBox(wtr("mediabrowser.share"), wtr("mediabrowser.share.dialog").arg(media.title(t)).arg(settings->shareLink(media.uid()).url()), NoIcon, Ok);
    shareMessageBox->button(Ok)->clicked().connect([=](WMouseEvent) { shareMessageBox->accept(); });
    shareMessageBox->show();
  } ));
  addWidget(header);
  addWidget(mediaInfoPanel.first);
  addWidget(actions.first);
  
  if(isAdmin) {
    auto adminActions = createPanel("mediabrowser.admin.actions");
    adminActions.first->setCollapsed(true);
    adminActions.second->addWidget(WW<WPushButton>(wtr("mediabrowser.admin.settitle")).css("btn btn-block btn-small btn-primary").onClick([=](WMouseEvent){ setTitle().emit(media);} ));
    adminActions.second->addWidget(WW<WPushButton>(wtr("mediabrowser.admin.setposter")).css("btn btn-block btn-small btn-primary").onClick([=](WMouseEvent){ setPoster().emit(media);} ));
    adminActions.second->addWidget(WW<WPushButton>(wtr("mediabrowser.admin.deletepreview")).css("btn btn-block btn-small btn-danger").onClick([=](WMouseEvent){ deletePoster().emit(media);} ));
    addWidget(adminActions.first);
  }
}


pair<WPanel*,WContainerWidget*> InfoPanel::createPanel(string titleKey)
{
  WPanel *panel = new WPanel();
  panel->setTitle(wtr(titleKey));
  panel->setCollapsible(true);
  panel->setMargin(10, Wt::Side::Top);
  panel->setAnimation({WAnimation::SlideInFromTop, WAnimation::EaseOut, 500});
  panel->titleBarWidget()->clicked().connect([=](WMouseEvent){ panel->setCollapsed(!panel->isCollapsed());});
  WContainerWidget *container = new WContainerWidget();
  panel->setCentralWidget(container);
  return {panel, container};
}


Wt::WContainerWidget *InfoPanel::labelValueBox(string label, Wt::WString value)
{
  return labelValueBox(label, new WText{value});
}


WContainerWidget* InfoPanel::labelValueBox(string label, WWidget* widget)
{
  WContainerWidget *box = WW<WContainerWidget>().css("row-fluid");
  box->addWidget(WW<WContainerWidget>().css("span3").add(WW<WText>(wtr(label)).css("label label-info")));
  box->addWidget(WW<WContainerWidget>().css("span9 media-info-value").setContentAlignment(AlignRight).add(widget));
  return box;
}



void MediaCollectionBrowserPrivate::browse(CollectionPath *currentPath)
{
  this->currentPath = currentPath;
  infoPanel->reset();
  browser->clear();
  rebuildBreadcrumb();
  currentPath->render([=](string key, CollectionPath *collectionPath){
    if(collectionPaths[key]) {
      delete collectionPaths[key];
      collectionPaths.erase(key);
    }
    collectionPaths[key] = collectionPath;
    addDirectory(collectionPath);
  }, [=](Media media){ addMedia(media);});
}

void DirectoryCollectionPath::render(OnDirectoryAdded directoryAdded, OnMediaAdded mediaAdded)
{
  auto belongsToCurrent = [=](fs::path p){
    return p.parent_path() == this->path;
  };
  
  set<fs::path> directories;
  vector<Media> medias;
  
  for(MediaEntry m: mediaCollection->collection()) {
    if(belongsToCurrent(m.second.path()))
      medias.push_back(m.second);
    if(m.second.filename().empty() || m.second.fullPath().empty()) continue;
    
    fs::path directory = m.second.parentDirectory();
    while(/*directory != collection->rootPath() && */!belongsToCurrent(directory) && directory != filesystem::path("/")) {
      directory = directory.parent_path();
    }
    if(directory != this->path && belongsToCurrent(directory) && !directories.count(directory))
      directories.insert(directory);
  }
  
  std::sort(medias.begin(), medias.end(), [](const Media &a, const Media &b)->bool{ return (a.filename() <b.filename()); } );
  
  for(fs::path directory: directories) {
    directoryAdded(directory.string(), new DirectoryCollectionPath{directory, mediaCollection, this});
  }
  for(Media media: medias) mediaAdded(media);
}

void RootCollectionPath::render(OnDirectoryAdded directoryAdded, OnMediaAdded mediaAdded)
{
  for(string directory: settings->mediasDirectories(session)) {
    directoryAdded(directory, new DirectoryCollectionPath{directory, mediaCollection, this});
  }
}


string DirectoryCollectionPath::label() const
{
  return path.filename().string();
}

string RootCollectionPath::label() const
{
  return wtr("mediacollection.root").toUTF8();
}



void MediaCollectionBrowserPrivate::addDirectory(CollectionPath* directory)
{
  auto onClick = [=](WMouseEvent){
    browse(directory);
  };
  addIcon(directory->label(), [](WObject*){ return Settings::icon(Settings::FolderBig); }, onClick);
}


void MediaCollectionBrowserPrivate::addMedia(Media &media)
{
  wApp->log("notice") << "adding media " << media.path();
  Dbo::Transaction t(*session);
  
  auto onClick = [=](WMouseEvent e){ infoPanel->info(media); };
  
  GetIconF icon = [](WObject *){ return Settings::icon(Settings::VideoFile); };
  if(media.mimetype().find("audio") != string::npos)
    icon = [](WObject *){ return Settings::icon(Settings::AudioFile); };
  
  Dbo::ptr<MediaAttachment> preview = media.preview(t, Media::PreviewThumb);
  if(preview)
    icon = [=](WObject *parent) { Dbo::Transaction t(*session); return preview->link(preview, t, parent).url(); };
  
  addIcon(media.title(t), icon, onClick);
}

void MediaCollectionBrowserPrivate::clearThumbnailsFor(Media media)
{
  Dbo::Transaction t(*session);
  session->execute("DELETE FROM media_attachment WHERE media_id=? and type = 'preview';").bind(media.uid());
  t.commit();
  q->reload();
}


void MediaCollectionBrowserPrivate::setPosterFor(Media media)
{
  // TODO: very messy... FFMPEGMedia is of course deleted, need to create a copy...
  FFMPEGMedia *ffmpegMedia = new FFMPEGMedia{media};
  WDialog *dialog = new WDialog(wtr("mediabrowser.admin.setposter"));
  auto createThumbs = new CreateThumbnails{wApp, settings, dialog};
  dialog->footer()->addWidget(WW<WPushButton>(wtr("button.cancel")).css("btn btn-danger").onClick([=](WMouseEvent) {
    dialog->reject();
    delete ffmpegMedia;
  }));
  dialog->footer()->addWidget(WW<WPushButton>(wtr("button.ok")).css("btn btn-success").onClick([=](WMouseEvent) {
    Dbo::Transaction t(*session);
    createThumbs->save(&t);
    t.commit();
    dialog->accept();
    q->reload();
    delete ffmpegMedia;
  }));
  dialog->show();
  dialog->resize(500, 500);
  auto runStep = [=] {
    Dbo::Transaction t(*session);
    createThumbs->run(ffmpegMedia, media, dialog->contents(), &t, MediaScannerStep::OverwriteIfExisting );
  };
  createThumbs->redo().connect([=](_n6) {
    runStep();
  });
  runStep();
}


void MediaCollectionBrowserPrivate::setTitleFor(Media media)
{
  Dbo::Transaction t(*session);
  MediaPropertiesPtr properties = media.properties(t);
  if(!properties) {
    t.rollback();
    WMessageBox::show(wtr("mediabrowser.admin.settitle.missingproperties.caption"), wtr("mediabrowser.admin.settitle.missingproperties.body"), StandardButton::Ok);
    return;
  }
  WDialog *setTitleDialog = new WDialog(wtr("mediabrowser.admin.settitle"));
  setTitleDialog->contents()->addStyleClass("form-inline");
  WLineEdit *titleEdit = new WLineEdit(properties->title().empty() ? media.filename() : properties->title());
  WPushButton* okButton = WW<WPushButton>("Ok").onClick([=](WMouseEvent) { setTitleDialog->accept(); } ).css("btn");
  auto editIsEnabled = [=] {
    return !titleEdit->text().empty() && titleEdit->text().toUTF8() != media.filename() && titleEdit->text().toUTF8() != properties->title();
  };
  okButton->setEnabled(editIsEnabled());
  
  titleEdit->keyWentUp().connect([=](WKeyEvent key){
    if(key.key() == Wt::Key_Enter && editIsEnabled() )
      setTitleDialog->accept();
    okButton->setEnabled(editIsEnabled());
  });
  setTitleDialog->contents()->addWidget(new WText{wtr("set.title.filename.hint")});
  string titleHint = Utils::titleHintFromFilename(media.filename());
  setTitleDialog->contents()->addWidget(WW<WAnchor>("", titleHint).css("link-hand").onClick([=](WMouseEvent){
    titleEdit->setText(titleHint);
    okButton->setEnabled(editIsEnabled());
  }));
  setTitleDialog->contents()->addWidget(new WBreak);
  setTitleDialog->contents()->addWidget(titleEdit);
  setTitleDialog->contents()->addWidget(okButton);
  setTitleDialog->contents()->setPadding(10);
  setTitleDialog->setClosable(true);
  titleEdit->setWidth(500);
  setTitleDialog->finished().connect([=](WDialog::DialogCode code, _n5){
    if(code != WDialog::Accepted)
      return;
    Dbo::Transaction t(*session);
    properties.modify()->setTitle(titleEdit->text().toUTF8());
    properties.flush();
    t.commit();
    q->reload();
  });
  setTitleDialog->show();
}


WContainerWidget* MediaCollectionBrowserPrivate::addIcon(WString filename, GetIconF icon, MouseEventListener onClick)
{
    WContainerWidget *item = WW<WContainerWidget>().css("span3 media-icon-container");
    item->setContentAlignment(AlignmentFlag::AlignCenter);
    WAnchor *link = WW<WAnchor>("").css("thumbnail filesystem-item link-hand");
    link->setImage(new WImage(icon(item) ));
    link->addWidget(WW<WText>(filename).css("filesystem-item-label"));
    item->addWidget(link);
    link->clicked().connect(onClick);

    browser->addWidget(item);
    return item;
}




void MediaCollectionBrowserPrivate::rebuildBreadcrumb()
{
  breadcrumb->clear();
  breadcrumb->addWidget(WW<WPushButton>(wtr("mediacollection.reload")).css("btn btn-small").onClick([=](WMouseEvent) {
    Dbo::Transaction t(*session);
    for(auto path: collectionPaths) {
      if(path.first == ROOT_PATH_ID) continue;
      delete path.second;
      collectionPaths.erase(path.first);
    }
    collection->rescan(t);
    browse(collectionPaths[ROOT_PATH_ID]);
  }));
  list<CollectionPath*> paths;
  CollectionPath *current = currentPath;
  while(current) {
    paths.push_front(current);
    current = current->parent();
  }
  
  for(CollectionPath *path: paths) {
    WContainerWidget *item = new WContainerWidget;
    if(breadcrumb->count())
      item->addWidget(WW<WText>("/").css("divider"));
    item->addWidget( WW<WAnchor>("", path->label()).css("link-hand").onClick([=](WMouseEvent){
      browse(path);
    }) );
    breadcrumb->addWidget(item);
  }
}

Signal< Media >& MediaCollectionBrowser::play()
{
  return d->playSignal;
}

Signal< Media >& MediaCollectionBrowser::queue()
{
  return d->queueSignal;
}



MediaCollectionBrowser::~MediaCollectionBrowser()
{
  for(auto collectionPath: d->collectionPaths) {
    delete collectionPath.second;
    d->collectionPaths.erase(collectionPath.first);
  }
  delete d;
}

