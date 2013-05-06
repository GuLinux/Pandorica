#include "mediacollectionbrowser.h"
#include <Wt/WMenu>
#include <Wt/WAnchor>
#include <Wt/WText>
#include <Wt/WPopupMenu>
#include <Wt/WImage>
#include <Wt/WMemoryResource>
#include "Wt-Commons/wt_helpers.h"
#include "settings.h"
#include "mediaattachment.h"
#include "session.h"
#include "sessioninfo.h"
#include "sessiondetails.h"
#include "comment.h"
#include "utils.h"
#include <boost/format.hpp>
#include <algorithm>
#include <Wt/Dbo/Transaction>
#include <Wt/WTime>
#include <Wt/WMessageBox>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>

using namespace Wt;
using namespace std;
using namespace boost;

namespace fs = boost::filesystem;

typedef pair<string,Media> MediaEntry;
typedef std::function<string(WObject*)> GetIconF;

struct Popover {
  WString title;
  WString text;
  bool isValid() const { return !title.empty() && !text.empty(); }
};

class MediaCollectionBrowserPrivate {
public:
  MediaCollectionBrowserPrivate(MediaCollection *collection, Settings *settings, Session *session, MediaCollectionBrowser *q)
    : collection(collection) , settings(settings), session(session), q(q) {}
    void rebuildBreadcrumb();
    void browse(filesystem::path currentPath);
public:
  bool isAdmin = false;
  MediaCollection *const collection;
  Settings *settings;
  Session *session;
  filesystem::path currentPath;
  WContainerWidget* breadcrumb;
  WContainerWidget* browser;
  Signal< Media > playSignal;
  Signal< Media > queueSignal;
  bool roleWasFetched = false;
private:
  void addDirectory(filesystem::path directory);
  void addMedia(Media& media);
  WContainerWidget* addIcon(WString filename, GetIconF icon, MouseEventListener onClick, Popover popover = Popover());
  string formatFileSize(long size);
    void setTitleFor(Media media);
    void clearThumbnailsFor(Media media);
  MediaCollectionBrowser* q;
};

MediaCollectionBrowser::MediaCollectionBrowser(MediaCollection* collection, Settings* settings, Session* session, WContainerWidget* parent)
  : WContainerWidget(parent), d(new MediaCollectionBrowserPrivate(collection, settings, session, this))
{
  d->breadcrumb = WW<WContainerWidget>().css("breadcrumb");
  d->breadcrumb->setList(true);
  d->browser = WW<WContainerWidget>().css("thumbnails").setMargin(WLength::Auto, Left).setMargin(WLength::Auto, Right);
  d->browser->setList(true);
  addWidget(d->breadcrumb);
  addWidget(d->browser);
  d->currentPath = d->collection->rootPath();
}

void MediaCollectionBrowser::reload()
{
  d->browse(d->currentPath);
}


void MediaCollectionBrowserPrivate::browse(filesystem::path currentPath)
{
  this->currentPath = currentPath;
  browser->clear();
  rebuildBreadcrumb();
  auto belongsToCurrent = [=](fs::path p){
    return p.parent_path() == this->currentPath;
  };

  set<fs::path> directories;
  vector<Media> medias;
  
  for(MediaEntry m: collection->collection()) {
    if(belongsToCurrent(m.second.path()))
      medias.push_back(m.second);
    if(m.second.filename().empty() || m.second.fullPath().empty()) continue;
    
    fs::path directory = m.second.parentDirectory();
    while(directory != collection->rootPath() && !belongsToCurrent(directory) && directory != filesystem::path("/")) {
      directory = directory.parent_path();
    }
    if(directory != currentPath && belongsToCurrent(directory) && !directories.count(directory))
      directories.insert(directory);
  }
  
  std::sort(medias.begin(), medias.end(), [](const Media &a, const Media &b)->bool{ return (a.filename() <b.filename()); } );
  
  for(fs::path directory: directories) addDirectory(directory);
  for(Media media: medias) addMedia(media);
}

void MediaCollectionBrowserPrivate::addDirectory(filesystem::path directory)
{
  auto onClick = [=](WMouseEvent){
    browse(directory);
  };
  addIcon(directory.filename().string(), [](WObject*){ return Settings::icon(Settings::FolderBig); }, onClick);
}


void MediaCollectionBrowserPrivate::addMedia(Media &media)
{
  wApp->log("notice") << "adding media " << media.path();
  Popover popover{media.title(session)};
  WString fileSizeText = wtr("mediabrowser.filesize").arg(formatFileSize(fs::file_size(media.path()) ) );
  popover.text += fileSizeText;
  Dbo::Transaction t(*session);
  
  if(!roleWasFetched) {
    roleWasFetched = true;
    Dbo::Transaction t(*session);
    isAdmin = session->user()->isAdmin();
  }
  
  MediaPropertiesPtr mediaProperties = session->find<MediaProperties>().where("media_id = ?").bind(media.uid());
  WString mediaLengthText;
  if(mediaProperties && mediaProperties->duration() > 0) {
    mediaLengthText = wtr("mediabrowser.medialength").arg( WTime(0,0,0).addSecs(mediaProperties->duration()).toString() );
    popover.text += "<br />" + mediaLengthText;
  }
  
  auto onClick = [=](WMouseEvent e){
    WPopupMenu *menu = new WPopupMenu();
    WPopupMenuItem* filename = menu->addItem(media.title(session));
    
    filename->setSelectable(false); filename->setDisabled(true);
    WPopupMenuItem* filesize = menu->addItem(fileSizeText);
    filesize->setSelectable(false); filesize->setDisabled(true);
    
    if(!mediaLengthText.empty()) {
      WPopupMenuItem* mediaLength = menu->addItem(mediaLengthText);
      mediaLength->setSelectable(false); mediaLength->setDisabled(true);
    }
    
    WPopupMenuItem* play = menu->addItem(wtr("mediabrowser.play"));
    WPopupMenuItem* queue = menu->addItem(wtr("mediabrowser.queue"));
    WPopupMenuItem* close = menu->addItem(wtr("mediabrowser.cancelpopup"));
    WPopupMenuItem* share = menu->addItem(wtr("mediabrowser.share"));
    WPopupMenuItem* clearThumbs = 0;
    WPopupMenuItem* setTitle = 0;
    if(isAdmin) {
      clearThumbs = menu->addItem(wtr("mediabrowser.admin.deletepreview"));
      setTitle = menu->addItem(wtr("mediabrowser.admin.settitle"));
    }
    
    menu->aboutToHide().connect([=](_n6){
      if(menu->result() == play)
        playSignal.emit(media);
      if(menu->result() == queue)
        queueSignal.emit(media);
      if(clearThumbs && menu->result() == clearThumbs) {
        clearThumbnailsFor(media);
      }
      if(menu->result() == share) {
        WMessageBox::show(wtr("mediabrowser.share"), wtr("mediabrowser.share.dialog").arg(media.title(session)).arg(settings->shareLink(media.uid()).url()), StandardButton::Ok);
      }
      if(setTitle && menu->result() == setTitle) {
        setTitleFor(media);
      }
    });
    menu->popup(e);
  };
  GetIconF icon = [](WObject *){ return Settings::icon(Settings::VideoFile); };
  if(media.mimetype().find("audio") != string::npos)
    icon = [](WObject *){ return Settings::icon(Settings::AudioFile); };
  Dbo::ptr<MediaAttachment> preview = media.preview(session, Media::PreviewThumb);
  if(preview)
    icon = [=](WObject *parent) {
      auto resource = new WMemoryResource(preview->mimetype(), preview->data(), parent);
//       resource->setInternalPath(wApp->sessionId() + "-preview-" + media.uid());
      return resource->url();
    };
  addIcon(media.title(session), icon, onClick, popover);
}

void MediaCollectionBrowserPrivate::clearThumbnailsFor(Media media)
{
  Dbo::Transaction t(*session);
  session->execute("DELETE FROM media_attachment WHERE media_id=? and type = 'preview';").bind(media.uid());
  t.commit();
  q->reload();
}


void MediaCollectionBrowserPrivate::setTitleFor(Media media)
{
  Dbo::Transaction t(*session);
  MediaPropertiesPtr properties = session->find<MediaProperties>().where("media_id = ?").bind(media.uid());
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


WContainerWidget* MediaCollectionBrowserPrivate::addIcon(WString filename, GetIconF icon, MouseEventListener onClick, Popover popover)
{
    WContainerWidget *item = WW<WContainerWidget>().css("span3 media-icon-container");
    item->setContentAlignment(AlignmentFlag::AlignCenter);
    WAnchor *link = WW<WAnchor>("").css("thumbnail filesystem-item link-hand");
    link->setImage(new WImage(icon(item) ));
    link->addWidget(WW<WText>(filename).css("filesystem-item-label"));
    item->addWidget(link);
    link->clicked().connect(onClick);
    if(popover.isValid()) {
      link->setAttributeValue("data-toggle", "popover");
      string tooltipJS = (boost::format(JS(
        var element = $('#%s');
        var positionLeft = element.offset()["left"];
        var positionRight = $(window).width() - positionLeft - element.width();
        var positionBottom = $(window).height() - element.offset()["top"] - element.height();
        console.log("Left: " + positionLeft + ", Right: " + positionRight + ", window width: " + $(window).width());
        var placement = "bottom";
        if(positionBottom < 150) placement = "top";
        if(positionLeft < 100) placement = "right";
        if(positionRight < 150) placement = "left";
        element.popover({placement: placement, html: true, title: %s, content: %s, trigger: 'hover'});
      )) % link->id() % popover.title.jsStringLiteral() % popover.text.jsStringLiteral() ).str();
      link->doJavaScript(tooltipJS);
      
      link->clicked().connect([=](WMouseEvent) { link->doJavaScript((boost::format(JS(
        $('#%s').popover('hide');
      )) % link->id() ).str() ); } );
    }
    browser->addWidget(item);
    return item;
}


string MediaCollectionBrowserPrivate::formatFileSize(long size)
{
  int maxSizeForUnit = 900;
  vector<string> units {"bytes", "KB", "MB", "GB"};
  double unitSize = size;
  
  for(string unit: units) {
    if(unitSize<maxSizeForUnit || unit == units.back()) {
      return (boost::format("%.2f %s") % unitSize % unit).str();
    }
    unitSize /= 1024;
  }
}



void MediaCollectionBrowserPrivate::rebuildBreadcrumb()
{
  breadcrumb->clear();
  breadcrumb->addWidget(WW<WPushButton>(wtr("mediacollection.reload")).css("btn btn-small").onClick([=](WMouseEvent) {
    collection->rescan();
    browse(collection->rootPath());
  }));
  list<fs::path> paths;
  fs::path p = currentPath;
  while(p != collection->rootPath()) {
    paths.push_front(p);
    p = p.parent_path();
  }
  paths.push_front(p);
  
  for(fs::path p: paths) {
    WContainerWidget *item = new WContainerWidget;
    if(breadcrumb->count())
      item->addWidget(WW<WText>("/").css("divider"));
    item->addWidget( WW<WAnchor>("", p.filename().string()).css("link-hand").onClick([=](WMouseEvent){
      browse(p);
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

}

