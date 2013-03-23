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

using namespace Wt;
using namespace std;
using namespace boost;

namespace fs = boost::filesystem;

typedef pair<string,Media> MediaEntry;

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
private:
  void addDirectory(filesystem::path directory);
  void addMedia(Media media);
  WContainerWidget* addIcon(string filename, string icon, MouseEventListener onClick);
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
  d->browse(d->collection->rootPath());
}

void MediaCollectionBrowserPrivate::browse(filesystem::path currentPath)
{
  this->currentPath = currentPath;
  browser->clear();
  rebuildBreadcrumb();
  auto belongsToCurrent = [this](fs::path p){
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
  auto onClick = [this,directory](WMouseEvent){
    browse(directory);
  };
  addIcon(directory.filename().string(), "http://gulinux.net/css/fs_icons/inode-directory.png", onClick);
}

void MediaCollectionBrowserPrivate::addMedia(Media media)
{
  wApp->log("notice") << "adding media " << media.path();
  auto onClick = [this,media](WMouseEvent e){
    WPopupMenu *menu = new WPopupMenu();
    WPopupMenuItem* filename = menu->addItem(media.filename());
    filename->setSelectable(false); filename->setDisabled(true);
    WPopupMenuItem* filesize = menu->addItem(string("File size: ") + formatFileSize(fs::file_size(media.path())));
    filesize->setSelectable(false); filesize->setDisabled(true);
    WPopupMenuItem* play = menu->addItem("Play");
    WPopupMenuItem* queue = menu->addItem("Queue");
    WPopupMenuItem* close = menu->addItem("Cancel");
    WPopupMenuItem* clearThumbs = 0;
    Dbo::Transaction t(*session);
    AuthorizedUserPtr authUser = session->find<AuthorizedUser>().where("email = ?").bind( session->login().user().email() );
    if(authUser && authUser->role() == AuthorizedUser::Admin) {
      clearThumbs = menu->addItem("Delete Preview");
    }
    
    menu->aboutToHide().connect([this,media,menu,play,queue,close,clearThumbs](_n6){
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
  string icon = "http://gulinux.net/css/fs_icons/video-x-generic.png";
  if(media.mimetype().find("audio") != string::npos)
    icon = "http://gulinux.net/css/fs_icons/audio-x-generic.png";
  Dbo::ptr<MediaAttachment> preview = media.preview(session, Media::PreviewThumb);
  if(preview)
    icon = (new WMemoryResource(preview->mimetype(), preview->data(), q))->url();;
  addIcon(media.filename(), icon, onClick);
}

WContainerWidget* MediaCollectionBrowserPrivate::addIcon(string filename, string icon, MouseEventListener onClick)
{
    WContainerWidget *item = WW(WContainerWidget).css("span3");
    item->setContentAlignment(AlignmentFlag::AlignCenter);
    WAnchor *link = WW(WAnchor, "javascript:false").css("thumbnail filesystem-item");
    link->setImage(new WImage(icon));
    link->addWidget(WW(WText, filename).css("filesystem-item-label"));
    item->addWidget(link);
    link->clicked().connect(onClick);
    
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
    item->addWidget( WW(WAnchor, "javascript:false", p.filename().string()).onClick([this,p](WMouseEvent){
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

