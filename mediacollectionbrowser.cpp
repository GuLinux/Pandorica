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
#include "authorizeduser.h"
#include <boost/format.hpp>
#include <algorithm>
#include <Wt/Dbo/Transaction>
#include <Wt/WTime>

using namespace Wt;
using namespace std;
using namespace boost;

namespace fs = boost::filesystem;

typedef pair<string,Media> MediaEntry;
typedef std::function<string(WObject*)> GetIconF;

struct Popover {
  WString title;
  WString text;
  Popover(WString title = WString(), WString text = WString()) : title(title), text(text) {}
  bool isValid() const { return !title.empty() && !text.empty(); }
};

class MediaCollectionBrowserPrivate {
public:
  MediaCollectionBrowserPrivate(MediaCollection *collection, Settings *settings, Session *session, MediaCollectionBrowser *q)
    : collection(collection) , settings(settings), session(session), q(q) {}
    void rebuildBreadcrumb();
    void browse(filesystem::path currentPath);
    
public:
  MediaCollection *const collection;
  Settings *settings;
  Session *session;
  filesystem::path currentPath;
  WContainerWidget* breadcrumb;
  WContainerWidget* browser;
  Signal< Media > playSignal;
  Signal< Media > queueSignal;
  bool isAdmin;
private:
  void addDirectory(filesystem::path directory);
  void addMedia(Media media);
  WContainerWidget* addIcon(string filename, GetIconF icon, MouseEventListener onClick, Popover popover = Popover());
  string formatFileSize(long size);
  MediaCollectionBrowser* q;
};

MediaCollectionBrowser::MediaCollectionBrowser(MediaCollection* collection, Settings* settings, Session* session, WContainerWidget* parent)
  : WContainerWidget(parent), d(new MediaCollectionBrowserPrivate(collection, settings, session, this))
{
  d->breadcrumb = WW(WContainerWidget).css("breadcrumb");
  d->breadcrumb->setList(true);
  d->browser = WW(WContainerWidget).css("thumbnails").setMargin(WLength::Auto, Left).setMargin(WLength::Auto, Right);
  d->browser->setList(true);
  addWidget(d->breadcrumb);
  addWidget(d->browser);
  d->currentPath = d->collection->rootPath();
  Dbo::Transaction t(*session);
  AuthorizedUserPtr authUser = session->find<AuthorizedUser>().where("email = ?").bind( session->login().user().email() );
  d->isAdmin = authUser && authUser->role() == AuthorizedUser::Admin;
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
    fs::path directory = m.second.parentDirectory();
    while(directory != collection->rootPath() && !belongsToCurrent(directory)) {
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
  addIcon(directory.filename().string(), [](WObject*){ return "http://gulinux.net/css/fs_icons/inode-directory.png"; }, onClick);
}

void MediaCollectionBrowserPrivate::addMedia(Media media)
{
  wApp->log("notice") << "adding media " << media.path();
  Popover popover(media.filename());
  WString fileSizeText = WString::tr("mediabrowser.filesize").arg(formatFileSize(fs::file_size(media.path()) ) );
  popover.text += fileSizeText;
  Dbo::Transaction t(*session);
  MediaPropertiesPtr mediaProperties = session->find<MediaProperties>().where("media_id = ?").bind(media.uid());
  WString mediaLengthText;
  if(mediaProperties && mediaProperties->duration() > 0) {
    mediaLengthText = WString::tr("mediabrowser.medialength").arg( WTime(0,0,0).addSecs(mediaProperties->duration()).toString() );
    popover.text += "<br />" + mediaLengthText;
  }
  
  auto onClick = [=](WMouseEvent e){
    WPopupMenu *menu = new WPopupMenu();

    WPopupMenuItem* filename = menu->addItem(media.filename());
    filename->setSelectable(false); filename->setDisabled(true);
    WPopupMenuItem* filesize = menu->addItem(fileSizeText);
    filesize->setSelectable(false); filesize->setDisabled(true);
    
    if(!mediaLengthText.empty()) {
      WPopupMenuItem* mediaLength = menu->addItem(mediaLengthText);
      mediaLength->setSelectable(false); mediaLength->setDisabled(true);
    }
    
    WPopupMenuItem* play = menu->addItem(WString::tr("mediabrowser.play"));
    WPopupMenuItem* queue = menu->addItem(WString::tr("mediabrowser.queue"));
    WPopupMenuItem* close = menu->addItem(WString::tr("mediabrowser.cancelpopup"));
    WPopupMenuItem* clearThumbs = 0;
    if(isAdmin) {
      clearThumbs = menu->addItem(WString::tr("mediabrowser.admin.deletepreview"));
    }
    
    menu->aboutToHide().connect([=](_n6){
      if(menu->result() == play)
        playSignal.emit(media);
      if(menu->result() == queue)
        queueSignal.emit(media);
      if(clearThumbs && menu->result() == clearThumbs) {
        Dbo::Transaction t(*session);
        session->execute("DELETE FROM media_attachment WHERE media_id=? and type = 'preview';").bind(media.uid());
        t.commit();
      }
    });
    menu->popup(e);
  };
  std::function<string(WObject*)> icon = [](WObject *){ return "http://gulinux.net/css/fs_icons/video-x-generic.png"; };
  if(media.mimetype().find("audio") != string::npos)
    icon = [](WObject *){ return "http://gulinux.net/css/fs_icons/audio-x-generic.png"; };
  Dbo::ptr<MediaAttachment> preview = media.preview(session, Media::PreviewThumb);
  if(preview)
    icon = [=](WObject *parent){ return (new WMemoryResource(preview->mimetype(), preview->data(), parent))->url(); };
  addIcon(media.filename(), icon, onClick, popover);
}

WContainerWidget* MediaCollectionBrowserPrivate::addIcon(string filename, GetIconF icon, MouseEventListener onClick, Popover popover)
{
    WContainerWidget *item = WW(WContainerWidget).css("span3 media-icon-container");
    item->setContentAlignment(AlignmentFlag::AlignCenter);
    WAnchor *link = WW(WAnchor, "javascript:false").css("thumbnail filesystem-item");
    link->setImage(new WImage(icon(item) ));
    link->addWidget(WW(WText, filename).css("filesystem-item-label"));
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
      item->addWidget(WW(WText, "/").css("divider"));
    item->addWidget( WW(WAnchor, "javascript:false", p.filename().string()).onClick([=](WMouseEvent){
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

